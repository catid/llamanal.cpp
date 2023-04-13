#include "walk_directory.hpp"
#include "logging.hpp"

#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <filesystem>

#define ENABLE_MMAP

#ifdef ENABLE_MMAP
    #include <boost/interprocess/file_mapping.hpp>
    #include <boost/interprocess/mapped_region.hpp>
#endif

namespace analysis {


//------------------------------------------------------------------------------
// Directory Walker

void walk_directory(
    const std::vector<SupportedFileType>& supported_file_types,
    const std::string& path,
    int subdirectory_depth)
{
    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        if (entry.is_regular_file()) {
            // Get the file extension
            std::string ext = entry.path().extension().string();
            if (ext.empty()) {
                continue;  // File has no extension
            }
            ext = ext.substr(1);  // Remove the dot from the extension
            ext = boost::algorithm::to_lower_copy(ext); // Normalize case to lower

            // Check if the file type is supported
            auto it = std::find_if(supported_file_types.begin(), supported_file_types.end(),
                [&ext](const SupportedFileType& fileType) {
                    return fileType.Extension == ext;
                });

            if (it == supported_file_types.end()) {
                // File type is not supported
                BOOST_LOG_TRIVIAL(debug) << "Skipping unsupported file: " << entry.path().string();
                continue;
            }

#ifdef ENABLE_MMAP
            // Get the file size
            std::size_t size_bytes = std::filesystem::file_size(entry.path());

            // Map the file to memory
            boost::interprocess::file_mapping mapping(entry.path().string().c_str(),
                                                       boost::interprocess::read_only);
            boost::interprocess::mapped_region region(mapping, boost::interprocess::read_only);

            // Call the handler with the file contents
            it->Handler(
                entry.path().string(),
                static_cast<const char*>(region.get_address()),
                size_bytes,
                subdirectory_depth
            );
#else
            // Call the handler with the file contents
            it->Handler(
                entry.path().string(),
                nullptr,
                0,
                subdirectory_depth
            );
#endif
        } else if (entry.is_directory()) {
            walk_directory(
                supported_file_types,
                entry.path().string(),
                subdirectory_depth + 1
            );
        }
    }
}


} // namespace analysis
