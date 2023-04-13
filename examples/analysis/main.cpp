#ifdef ENABLE_CPP_SUPPORT
    #include "cpp_analysis.hpp"
#endif // ENABLE_CPP_SUPPORT

#include <iostream>
#include <string>
#include <unordered_map>
#include <filesystem>
#include <stdexcept>
#include <functional>
#include <vector>
#include <memory>
#include <boost/program_options.hpp>

// Placeholder for airate_cpp
std::string airate_cpp(const std::string& file_path) {
    return "";
}

std::string rate_file(const std::string& file_path, int depth, const std::string& node, int port) {
    std::string markdown_str;
    std::string file_ext = std::filesystem::path(file_path).extension().string();
    
    static std::unordered_map<std::string, std::function<std::string(const std::string&)>> file_handlers = {
        {".cpp", airate_cpp}
    };

    auto handler = file_handlers.find(file_ext);
    if (handler != file_handlers.end()) {
        markdown_str += std::string(depth * 2, ' ') + "* " + file_path + "\n";
        markdown_str += handler->second(file_path);
    }

    return markdown_str;
}

std::string process_directory(const std::string& path, const std::string& node, int port, int depth = 0) {
    std::string markdown_str;

    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        if (entry.is_regular_file()) {
            markdown_str += rate_file(entry.path().string(), depth, node, port);
        } else if (entry.is_directory()) {
            markdown_str += std::string(depth * 2, ' ') + "* " + entry.path().filename().string() + "/\n";
            markdown_str += process_directory(entry.path().string(), node, port, depth + 1);
        }
    }

    return markdown_str;
}

#include <boost/program_options.hpp>
#include <iostream>
#include <filesystem>

namespace po = boost::program_options;

int main(int argc, char* argv[]) {
    try {
        po::options_description desc("Allowed options");
        desc.add_options()
            ("help", "Print usage")
            ("path", po::value<std::string>(), "Path to the directory or file")
            ("node", po::value<std::string>()->default_value("localhost"), "Server node")
            ("port", po::value<int>()->default_value(5000), "Server port");

        po::positional_options_description positional;
        positional.add("path", 1);

        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).options(desc).positional(positional).run(), vm);
        po::notify(vm);

        if (vm.count("help") || !vm.count("path")) {
            std::cout << desc << std::endl;
            return 0;
        }

        std::string path = vm["path"].as<std::string>();
        std::string node = vm["node"].as<std::string>();
        int port = vm["port"].as<int>();

        if (std::filesystem::is_regular_file(path)) {
            std::cout << rate_file(path, 0, node, port) << std::endl;
        } else if (std::filesystem::is_directory(path)) {
            std::cout << process_directory(path, node, port) << std::endl;
        } else {
            std::cerr << "Error: " << path << " is not a valid file or directory" << std::endl;
        }
    } catch (const po::error& e) {
        std::cerr << "Error parsing options: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
