#pragma once

#include "CommandType.hpp"
#include <fstream>
#include <string>

namespace VMTranslator {
    class Parser {
    public:
        Parser(const std::string& fileName);
        bool        hasMoreCommands() const;
        void        advance();
        CommandType commandType() const;
        std::string arg1() const;
        int         arg2() const;
        std::string line() const;
    private:
        void toLower(std::string&) const;
        bool isNumeric(const std::string&) const;

        std::ifstream inFile_;
        bool hasMoreCommands_;
        std::string line_;
        CommandType commandType_;
        std::string arg1_;
        int arg2_;
    };
}
