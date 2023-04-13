#include "logging.hpp"
#include "walk_directory.hpp"

#ifdef ENABLE_CPP_SUPPORT
    #include "cpp_analysis.hpp"
#endif // ENABLE_CPP_SUPPORT

#include <boost/program_options.hpp>
#include <boost/scope_exit.hpp>
#include <boost/filesystem.hpp>

#define DEFAULT_MODEL "../models/ggml-LLaMa-65B-q4_0.bin"

using namespace analysis;


//------------------------------------------------------------------------------
// Application

void main_analysis(const std::string& path_, const std::string& model_)
{
    // Expand ~ and .. type stuff
    BOOST_LOG_TRIVIAL(debug) << "Input path: " << path_;
    BOOST_LOG_TRIVIAL(debug) << "Input model: " << model_;
    std::string path = boost::filesystem::canonical(path_).string();
    std::string model = boost::filesystem::canonical(model_).string();
    BOOST_LOG_TRIVIAL(debug) << "Canonicalized input path: " << path;
    BOOST_LOG_TRIVIAL(debug) << "Canonicalized input model: " << model;

    int files_checked = 0;

    std::vector<SupportedFileType> supported_file_types;

#ifdef ENABLE_CPP_SUPPORT
    BOOST_LOG_TRIVIAL(debug) << "Enabled C++ support.";

    auto cpp_handler = [&](
        const std::string& file_path,
        const char* file_contents,
        std::size_t file_length_in_bytes,
        int subdirectory_depth)
    {
        ++files_checked;
        BOOST_LOG_TRIVIAL(info) << std::string(subdirectory_depth * 2, ' ') << "* C++: " << file_path;

        int functions_checked = 0;

        auto func_handler = [&](const std::string &code) {
            ++functions_checked;
            BOOST_LOG_TRIVIAL(debug) << "Function from " << file_path << ":\n```cpp\n" << code << "\n```";
        };

        extract_cpp_functions(file_path, file_contents, file_length_in_bytes, func_handler);

        BOOST_LOG_TRIVIAL(debug) << std::string(subdirectory_depth * 2, ' ') << "* " << functions_checked << " functions from " << file_path;
    };

    supported_file_types.push_back({"cc", cpp_handler});
    supported_file_types.push_back({"hh", cpp_handler});
    supported_file_types.push_back({"ii", cpp_handler});
    supported_file_types.push_back({"inl", cpp_handler});
    supported_file_types.push_back({"cpp", cpp_handler});
    supported_file_types.push_back({"cxx", cpp_handler});
    supported_file_types.push_back({"hpp", cpp_handler});
    supported_file_types.push_back({"hxx", cpp_handler});
    supported_file_types.push_back({"c", cpp_handler});
    supported_file_types.push_back({"h", cpp_handler});
#endif // ENABLE_CPP_SUPPORT

    // Recursively check all files in the directory
    walk_directory(supported_file_types, path);

    if (files_checked <= 0) {
        BOOST_LOG_TRIVIAL(warning) << "No supported source files found in " << path;
    } else {
        BOOST_LOG_TRIVIAL(info) << "Analysis complete!  Checked " << files_checked << " files in " << path;
    }
}


//------------------------------------------------------------------------------
// Entrypoint

namespace po = boost::program_options;

int main(int argc, char* argv[]) {
    // Call stop_logging() before terminating.
    BOOST_SCOPE_EXIT_ALL() {
        stop_logging();
    };

    try {
        po::options_description desc("Available options");
        desc.add_options()
            ("help,h", "Print usage")
            ("verbose,v", "Verbose output")
            ("path,p", po::value<std::string>(), "Path to the directory or file")
            ("model,m", "Path to the model file.  Default: " DEFAULT_MODEL)
        ;

        po::positional_options_description positional;
        positional.add("path", 1);

        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).options(desc).positional(positional).run(), vm);
        po::notify(vm);

        std::string path = vm.count("path") > 0 ? vm["path"].as<std::string>() : "";
        std::string model = vm.count("model") > 0 ? vm["model"].as<std::string>() : DEFAULT_MODEL;
        bool verbose = vm.count("verbose") > 0;

        init_logging(verbose);

        BOOST_LOG_TRIVIAL(info) << "analysis :: Static code analysis with AI.";

        if (vm.count("help") || path.empty()) {
            BOOST_LOG_TRIVIAL(info) << "Please specify a file or directory to scan!";
            BOOST_LOG_TRIVIAL(info) << desc;
            return -1;
        }

        main_analysis(path, model);
    } catch (const po::error& e) {
        BOOST_LOG_TRIVIAL(error) << "Error parsing options: " << e.what() << std::endl;
        return -2;
    }

    return 0;
}
