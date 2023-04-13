#ifndef WALK_DIRECTORY_HPP
#define WALK_DIRECTORY_HPP

#include <vector>
#include <string>
#include <functional>

namespace analysis {


//------------------------------------------------------------------------------
// Directory Walker

// User-defined function that handles a file in the path
using FileHandler = std::function<void(
    const std::string& file_path,
    const char* file_contents,
    std::size_t file_length_in_bytes,
    int subdirectory_depth)>;

struct SupportedFileType
{
    std::string Extension;
    FileHandler Handler;
};

void walk_directory(
    const std::vector<SupportedFileType>& supported_file_types,
    const std::string& path,
    int subdirectory_depth = 0);


} // namespace analysis

#endif // WALK_DIRECTORY_HPP
