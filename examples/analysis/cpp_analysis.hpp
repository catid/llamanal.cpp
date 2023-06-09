#ifndef CPP_ANALYZE_HPP
#define CPP_ANALYZE_HPP

#include <vector>
#include <string>
#include <functional>

namespace analysis {


//------------------------------------------------------------------------------
// AST Parsing

// Extract all CPP functions from a file provided as a memory buffer
void extract_cpp_functions(
    std::string file_path,
    const char* file_contents,
    size_t size,
    std::function<void(const std::string &)> func_processor);


//------------------------------------------------------------------------------
// Prompt Generation

// Generates a string prompt to rate C++ code from 0..1
void ask_cpp_expert_score(
    std::string& out_prompt,
    std::vector<std::string>& stop_strs,
    const std::string& code,
    const std::string& user_role_ = "Human",
    const std::string& assistant_role_ = "Expert");


} // namespace analysis

#endif // CPP_ANALYZE_HPP
