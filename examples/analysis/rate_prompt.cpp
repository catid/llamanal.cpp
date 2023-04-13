#include "rate_prompt.hpp"
#include "logging.hpp"

#include <iostream>
#include <regex>
#include <string>
#include <vector>

namespace analysis {


//------------------------------------------------------------------------------
// Conversation Template

bool has_system_role(const std::vector<Message>& messages) {
    for (const auto& message : messages) {
        if (message.role == "system") {
            return true;
        }
    }
    return false;
}

std::string normalize_role(const std::string& role) {
    std::string capitalized_role = role;
    capitalized_role[0] = std::toupper(capitalized_role[0]);
    return capitalized_role;
}

std::string decorate_role(const std::string& role) {
    return "[|" + normalize_role(role) + "|]";
}

void create_conversation_template(
    std::string& out_prompt,
    std::vector<std::string>& out_stop_strs,
    const std::vector<Message>& messages,
    const std::string& custom_start,
    const std::string& user_role,
    const std::string& assistant_role)
{
    std::vector<std::string> conversation;

    for (const auto& message : messages) {
        const std::string& role = message.role;
        const std::string& content = message.content;

        if (normalize_role(role) == "System") {
            conversation.push_back(content);
        } else {
            conversation.push_back(decorate_role(role) + ": " + content);
        }
    }

    if (!custom_start.empty()) {
        conversation.push_back(decorate_role(assistant_role) + ": " + custom_start);
    } else {
        conversation.push_back(decorate_role(assistant_role) + ":");
    }

    out_stop_strs = {
        decorate_role(user_role),
        decorate_role(assistant_role)
    };

    out_prompt = "";
    for (const auto& line : conversation) {
        out_prompt += line + "\n";
    }
}



//------------------------------------------------------------------------------
// Output Parsing

bool find_first_number_between_0_and_1(const std::string& s, float& out_found)
{
    std::regex pattern(R"((?<![0-9.])0(\.\d+)?(?!\d)|(?<![0-9.])1(\.\d+)?(?!\d))");
    std::smatch matches;

    for (auto it = std::sregex_iterator(s.begin(), s.end(), pattern); it != std::sregex_iterator(); ++it) {
        auto match = *it;
        int start_index = match.position();
        if (start_index == 0 || s[start_index - 1] != '-') {
            float f = std::stof(match.str());
            if (f >= 0 && f <= 1) {
                out_found = f;
                return true;
            }
        }
    }

    return false;
}

bool is_number_complete(const std::string& s)
{
    if (s.empty()) {
        return false;
    }

    char last_char = s.back();
    if (last_char != '.' && !std::isdigit(last_char)) {
        return true;
    }

    return false;
}


} // namespace analysis
