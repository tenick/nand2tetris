#include "VMTranslator/CodeWriter.hpp"
#include "VMTranslator/Parser.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <filesystem>
#include <string_view>
#include <vector>

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Should have exactly 1 argument\n";
        return EXIT_FAILURE;
    }

    try {
        std::string pathStr = argv[1], outputFileName;
        std::vector<std::string> vmFilePaths;
        if (fs::is_regular_file(pathStr)) {
            outputFileName = fs::path(pathStr).replace_extension(".asm");
            vmFilePaths.push_back(pathStr);
        }
        else if (fs::is_directory(pathStr)){
            fs::path p(pathStr);
    
            // If the path ends with a slash (e.g. "dir/"), parent_path() gets the actual folder name
            std::string folderName = p.filename().empty() ? p.parent_path().filename().string() 
                                                        : p.filename().string();

            std::string dirName = folderName + ".asm";
            outputFileName = (p / dirName).string();

            for (const auto& entry : fs::directory_iterator(pathStr)) {
                if (!entry.is_regular_file() || entry.path().extension() != ".vm")
                    continue;
                vmFilePaths.push_back(entry.path().string());
            }
        }
        else {
            throw std::runtime_error("The input must be a file or a directory.\n");
        }

        VMTranslator::CodeWriter codeWriter(outputFileName, vmFilePaths.size() > 1);
        for (const auto& vmFilePath : vmFilePaths) {
            VMTranslator::Parser parser(vmFilePath);
            codeWriter.setFileName(vmFilePath);
            while (parser.hasMoreCommands()) {
                codeWriter.writeComment(parser.line());
                codeWriter.writeCommand(parser.commandType(), parser.arg1(), parser.arg2());
                parser.advance();
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
