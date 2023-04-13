#include "cpp_analysis.hpp"

#include "rate_prompt.hpp"

#include <fstream>
#include <functional>
#include <sstream>
#include <string>
#include <vector>
#include <clang-c/Index.h>

namespace analysis {


//------------------------------------------------------------------------------
// AST Parsing

static bool is_function(CXCursorKind kind) {
    return kind == CXCursor_CXXMethod || kind == CXCursor_FunctionDecl;
}

CXChildVisitResult visitor(CXCursor cursor, CXCursor /*parent*/, CXClientData client_data) {
    auto *data = static_cast<std::vector<CXCursor>*>(client_data);
    CXSourceLocation location = clang_getCursorLocation(cursor);
    CXFile file;
    clang_getSpellingLocation(location, &file, nullptr, nullptr, nullptr);

    if (file && is_function(clang_getCursorKind(cursor))) {
        data->push_back(clang_getCursorReferenced(cursor));
    }

    return CXChildVisit_Recurse;
}

std::string get_code_block(const CXCursor &cursor, const std::string &file_contents) {
    CXSourceRange range = clang_getCursorExtent(cursor);
    CXSourceLocation start = clang_getRangeStart(range);
    CXSourceLocation end = clang_getRangeEnd(range);

    unsigned start_offset, end_offset;
    clang_getSpellingLocation(start, nullptr, nullptr, nullptr, &start_offset);
    clang_getSpellingLocation(end, nullptr, nullptr, nullptr, &end_offset);

    std::string code_block = file_contents.substr(start_offset, end_offset - start_offset);
    return code_block;
}

std::vector<CXCursor> extract_functions_from_memory(const char* file_contents, size_t size) {
    CXIndex index = clang_createIndex(0, 0);
    CXUnsavedFile unsaved_file = { "in_memory_file.cpp", file_contents, static_cast<unsigned long>(size) };
    CXTranslationUnit tu = clang_parseTranslationUnit(index, "in_memory_file.cpp", nullptr, 0, &unsaved_file, 1, CXTranslationUnit_None);

    std::vector<CXCursor> functions;
    clang_visitChildren(clang_getTranslationUnitCursor(tu), visitor, &functions);

    clang_disposeTranslationUnit(tu);
    clang_disposeIndex(index);

    return functions;
}

void extract_cpp_functions(
    const char* file_contents,
    size_t size,
    std::function<void(const std::string &)> func_processor)
{
    std::string file_contents_str(file_contents, size);
    std::vector<CXCursor> functions = extract_functions_from_memory(file_contents, size);

    for (const auto &function : functions) {
        std::string code_block = get_code_block(function, file_contents_str);
        func_processor(code_block);
    }
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
