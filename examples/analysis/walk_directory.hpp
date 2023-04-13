#ifndef WALK_DIRECTORY_HPP
#define WALK_DIRECTORY_HPP

#include <vector>
#include <string>
#include <functional>

namespace analysis {


//------------------------------------------------------------------------------
// Directory Walker

// User-defined function that handles a file in the path
using FileHandler = std::function<void(const std::string& file_path, int depth)>;

void walk_directory(
    FileHandler handler,
    const std::vector<std::string>& supported_file_types,
    const std::string& path,
    int depth = 0);


} // namespace analysis

#endif // WALK_DIRECTORY_HPP
