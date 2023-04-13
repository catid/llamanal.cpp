#include "cpp_analysis.hpp"
#include "logging.hpp"
#include "rate_prompt.hpp"

#include <fstream>
#include <functional>
#include <sstream>
#include <string>
#include <vector>
#include <numeric>

#include <clang-c/Index.h>

namespace analysis {


//------------------------------------------------------------------------------
// AST Parsing

struct VisitorClientData
{
    std::vector<CXCursor> FunctionCursors;
    std::string ExpectedFilePath;
};

bool has_function_body(CXCursor cursor) {
    bool has_body = false;
    clang_visitChildren(cursor, [](CXCursor child, CXCursor /*parent*/, CXClientData client_data) {
        bool *has_body = reinterpret_cast<bool *>(client_data);
        if (clang_getCursorKind(child) == CXCursorKind::CXCursor_CompoundStmt) {
            *has_body = true;
            return CXChildVisit_Break;
        }
        return CXChildVisit_Continue;
    }, &has_body);

    return has_body;
}

void functions_in_file(VisitorClientData* data, CXCursor cursor) {
    auto cursor_visitor = [](CXCursor cursor, CXCursor /*parent*/, CXClientData client_data) {
        VisitorClientData* data = reinterpret_cast<VisitorClientData*>(client_data);

        // If is a class member or free function:
        auto cursor_kind = clang_getCursorKind(cursor);
        if (cursor_kind == CXCursorKind::CXCursor_FunctionDecl || cursor_kind == CXCursorKind::CXCursor_CXXMethod) {
            // If not just a prototype:
            if (has_function_body(cursor)) {
                // Get cursor position
                CXFile cursor_file;
                unsigned line, column, offset;
                clang_getSpellingLocation(clang_getCursorLocation(cursor), &cursor_file, &line, &column, &offset);

                const char* file_path = clang_getCString(clang_getFileName(cursor_file));

                if (data->ExpectedFilePath == file_path) {
                    data->FunctionCursors.push_back(cursor);
                }
            }
        } else {
            functions_in_file(data, cursor);
        }

        return CXChildVisit_Continue;
    };

    clang_visitChildren(cursor, cursor_visitor, data);
}

std::string function_source(const CXCursor& node, const std::string& file_contents) {
    CXSourceRange extent = clang_getCursorExtent(node);
    CXSourceLocation start = clang_getRangeStart(extent);
    CXSourceLocation end = clang_getRangeEnd(extent);

    unsigned start_offset, end_offset;
    clang_getSpellingLocation(start, nullptr, nullptr, nullptr, &start_offset);
    clang_getSpellingLocation(end, nullptr, nullptr, nullptr, &end_offset);

    std::string func_src = file_contents.substr(start_offset, end_offset - start_offset);

    std::vector<std::string> lines;
    std::istringstream iss(file_contents.substr(0, start_offset));
    for (std::string line; std::getline(iss, line);) {
        lines.push_back(line);
    }

    std::vector<std::string> comments;
    for (auto it = lines.rbegin(); it != lines.rend(); ++it) {
        std::string stripped = *it;
        stripped.erase(0, stripped.find_first_not_of(" \t\n\v\f\r"));
        stripped.erase(stripped.find_last_not_of(" \t\n\v\f\r") + 1);

        if (stripped.rfind("//", 0) == 0 || stripped.rfind("/*", 0) == 0 || stripped.rfind("*/", 0) == stripped.size() - 2) {
            comments.push_back(*it);
        } else {
            break;
        }
    }

    std::reverse(comments.begin(), comments.end());
    std::string comment_str = std::accumulate(comments.begin(), comments.end(), std::string(),
        [](const std::string& a, const std::string& b) {
            return a + (a.length() > 0 ? "\n" : "") + b;
        });

    return (comment_str.empty() ? "" : comment_str + "\n") + func_src;
}

void extract_cpp_functions(
    std::string file_path,
    const char* file_contents,
    size_t size,
    std::function<void(const std::string &)> func_processor)
{
    CXIndex index = clang_createIndex(0, 0);

    // Create an unsaved file with mmap content
    // I checked with `strace` and this is actually helping - mmap is being used for the source files.
    CXUnsavedFile unsaved_file;
    unsaved_file.Filename = file_path.c_str();
    unsaved_file.Contents = file_contents;
    unsaved_file.Length = size;

    CXTranslationUnit tu = clang_parseTranslationUnit(index, file_path.c_str(), nullptr, 0, &unsaved_file, 1, CXTranslationUnit_None);

    VisitorClientData client_data;
    client_data.ExpectedFilePath = file_path;
    functions_in_file(&client_data, clang_getTranslationUnitCursor(tu));
    for (const auto& cursor : client_data.FunctionCursors) {
        std::string code = function_source(cursor, file_contents);
        func_processor(code);
    }

    clang_disposeTranslationUnit(tu);
    clang_disposeIndex(index);
}


//------------------------------------------------------------------------------
// Prompt Generation

void ask_cpp_expert_score(
    std::string& out_prompt,
    std::vector<std::string>& out_stop_strs,
    const std::string& code,
    const std::string& user_role_,
    const std::string& assistant_role_)
{
    std::string user_role = normalize_role(user_role_);
    std::string assistant_role = normalize_role(assistant_role_);

    std::vector<Message> messages = {
        {"System", "The following is a C++ conversation between " + user_role + " and " + assistant_role + ". " + user_role + " and " + assistant_role + " take turns chatting. " + assistant_role + " always considers the previous query carefully. " + assistant_role + " always provides an expert rating from 0 to 1 of the provided C++ code."},
        {user_role, "Please rate the following C++ function from 0 to 1, where 0 means the code has a bug and 1 means the code cannot be improved:\n"
                    "// Function to calculate the factorial of a positive integer using recursion\n"
                    "int factorial(int n) {\n"
                    "    // Base case: If n is 0 or 1, the factorial is 1\n"
                    "    if (n == 0 || n == 1) {\n"
                    "        return 1;\n"
                    "    }\n"
                    "    // Recursive case: Calculate the factorial of (n-1) and multiply it by n\n"
                    "    else {\n"
                    "        return n * factorial(n-1);\n"
                    "    }\n"
                    "}\n"},
        {assistant_role, "After careful consideration, I would rate the given code as 1, meaning that it cannot be improved as it is a valid and functional implementation of the factorial function in C++."},
        {user_role, "Please rate the following C++ function from 0 to 1, where 0 means the code has a bug and 1 means the code cannot be improved:\n"
                    "// Sort an array of integers using bubble sort\n"
                    "void bubble_sort(int arr[], int n) {\n"
                    "    bool swapped;\n"
                    "    for (int i = 0; i < n - 1; i++) {\n"
                    "        swapped = false;\n"
                    "        for (int j = 0; j < n - i - 1; j += 2) {\n"
                    "            if (arr[j] > arr[j + 2]) {\n"
                    "                swap(arr[j], arr[j + 2]);\n"
                    "                swapped = true;\n"
                    "            }\n"
                    "        }\n"
                    "        if (swapped == false) {\n"
                    "            break;\n"
                    "        }\n"
                    "    }\n"
                    "}\n"},
        {assistant_role, "After careful consideration, I would rate the given code as 0, meaning it has a bug that needs to be fixed."},
        {user_role, "Please rate the following C++ function from 0 to 1, where 0 means the code has a bug and 1 means the code cannot be improved:\n" + code},
    };

    std::string custom_start = "After careful consideration, I would rate the given code as ";

    return create_conversation_template(out_prompt, out_stop_strs, messages, custom_start, user_role, assistant_role);
}


} // namespace analysis
