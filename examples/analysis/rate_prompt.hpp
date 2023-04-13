#ifndef RATE_PROMPT_HPP
#define RATE_PROMPT_HPP

#include <vector>
#include <string>

namespace analysis {


//------------------------------------------------------------------------------
// Conversation Template

struct Message {
    // "System", "User", "Expert", "AI", etc
    std::string role;

    // The message content from this role.
    std::string content;
};

// Capitalize the role like: user -> User
std::string normalize_role(const std::string& role);

// Decorate the role with [|Role|]
std::string decorate_role(const std::string& role);

// Helper: Create a string prompt for an LLM given the input messages.
void create_conversation_template(
    std::string& out_prompt,
    std::vector<std::string>& out_stop_strs,
    const std::vector<Message>& messages,
    const std::string& custom_start = "",
    const std::string& user_role = "Human",
    const std::string& assistant_role = "Assistant");


//------------------------------------------------------------------------------
// Output Parsing

// Returns true if a number was found, and sets `found` to the value.
// Returns false if no number was found.
bool find_first_number_between_0_and_1(const std::string& s, float& out_found);


} // namespace analysis

#endif // RATE_PROMPT_HPP
