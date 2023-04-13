#include "rate_prompt.hpp"
#include "logging.hpp"

#include <iostream>
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

bool find_first_number_between_0_and_1(const std::string &s, float &out_found) {
    BOOST_LOG_TRIVIAL(info) << "INPUT = '" << s << "'";

    size_t i = 0;
    size_t length = s.length();
    bool found_number = false;

    while (i < length) {
        if ((i == 0 || (!isdigit(s[i - 1]) && s[i - 1] != '.')) && (s[i] == '0' || s[i] == '1')) {
            size_t start_index = i;
            bool decimal_point_found = false;
            i++;

            while (i < length && (isdigit(s[i]) || (!decimal_point_found && s[i] == '.'))) {
                if (s[i] == '.') {
                    decimal_point_found = true;
                }
                i++;
            }

            if ((i == length || !isdigit(s[i])) && (start_index == 0 || s[start_index - 1] != '-')) {
                std::string number_str = s.substr(start_index, i - start_index);
                float f = std::stof(number_str);

                if (f >= 0 && f <= 1) {
                    out_found = f;
                    found_number = true;
                    break;
                }
            }
        } else {
            i++;
        }
    }

    return found_number;
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
