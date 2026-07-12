#include "VMTranslatorAlpha/CodeWriter.hpp"
#include "VMTranslatorAlpha/Parser.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Should have exactly 1 argument\n";
        return EXIT_FAILURE;
    }

    try {
        std::string filePath = argv[1];
        VMTranslator::Parser parser(filePath);
        VMTranslator::CodeWriter codeWriter(filePath);
        while (parser.hasMoreCommands()) {
            codeWriter.writeComment(parser.line());
            codeWriter.writeCommand(parser.commandType(), parser.arg1(), parser.arg2());
            parser.advance();
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
