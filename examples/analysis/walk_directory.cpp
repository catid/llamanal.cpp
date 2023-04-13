#include "walk_directory.hpp"

#include <filesystem>

namespace analysis {


//------------------------------------------------------------------------------
// Directory Walker

void walk_directory(
    FileHandler handler,
    const std::vector<std::string>& supported_file_types,
    const std::string& path,
    int depth)
{
    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        if (entry.is_regular_file()) {
            handler(entry.path().string(), depth);
        } else if (entry.is_directory()) {
            process_directory(handler, supported_file_types, entry.path().string(), depth + 1);
        }
    }
}


} // namespace analysis
