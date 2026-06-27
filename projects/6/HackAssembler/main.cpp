#include "HackAssembler.h"
#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Must contain exactly 1 argument: the file to assemble.\n";
        return EXIT_FAILURE;
    }

    try {
        std::string fileName = argv[1];
        HackAssembler hackAssembler{fileName};
        hackAssembler.assemble();
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

