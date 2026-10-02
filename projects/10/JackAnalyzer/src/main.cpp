#include <cstdlib>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <filesystem>
#include <string_view>
#include <vector>

#include "JackAnalyzer/CompilationEngine.hpp"

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Error: Should have exactly 1 argument\n";
        return EXIT_FAILURE;
    }

    try {
        std::string pathStr = argv[1];
        std::vector<std::string> jackFilePaths;
        if (fs::is_regular_file(pathStr)) {
            jackFilePaths.push_back(pathStr);
        }
        else if (fs::is_directory(pathStr)){
            fs::path p(pathStr);

            for (const auto& entry : fs::directory_iterator(pathStr)) {
                if (!entry.is_regular_file() || entry.path().extension() != ".jack")
                    continue;
                jackFilePaths.push_back(entry.path().string());
            }
        }
        else {
            throw std::runtime_error("The input must be a file or a directory.\n");
        }

        for (std::string_view jackFilePath : jackFilePaths) {
            JackAnalyzer::CompilationEngine compilationEngine(jackFilePath);
            compilationEngine.compileClass();
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
