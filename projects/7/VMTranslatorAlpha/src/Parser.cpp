#include "VMTranslatorAlpha/ArithmeticCommand.hpp"
#include "VMTranslatorAlpha/CommandType.hpp"
#include "VMTranslatorAlpha/LogicCommand.hpp"
#include "VMTranslatorAlpha/Parser.hpp"

#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace VMTranslator {
    Parser::Parser(const std::string& fileName) {
        inFile_.open(fileName);
        if (!inFile_.is_open()) {
            throw std::runtime_error("Failed to open '" + fileName + "'.\n");
        }

        advance();
    }

    bool Parser::hasMoreCommands() const {
        return hasMoreCommands_;
    }

    CommandType Parser::commandType() const {
        return commandType_;
    }

    void Parser::advance() {
        std::string line, newLine;
        std::vector<std::string> tokens;
        while (std::getline(inFile_, line)) {
            std::istringstream iss(line);
            std::string word;
            while (iss >> word) {
                if (word == "//") break;
                tokens.push_back(word);
                newLine += word + " ";
            }
            if (!tokens.empty()) break;
        }
        line_ = newLine;
        hasMoreCommands_ = !tokens.empty();
        if (!hasMoreCommands_) return;
        line_.pop_back();

        // process statement
        std::string command = tokens[0];
        toLower(command);

        if (command == "push" || command == "pop") {
            commandType_ = command == "push" ? CommandType::C_PUSH : CommandType::C_POP;
            if (tokens.size() != 3) {
                throw std::runtime_error("'" + command + "' command requires exactly 3 arguments");
            }
            if (!isNumeric(tokens[2])) {
                throw std::runtime_error("'" + command + "' command requires a numeric 2nd argument");
            }
            arg1_ = tokens[1];
            toLower(arg1_);
            arg2_ = std::stoi(tokens[2]);
        }
        else if (isStringAnArithmeticCommand(command)) {
            commandType_ = CommandType::C_ARITHMETIC;
            arg1_ = command;
            toLower(arg1_);
        }
        else if (isStringALogicCommand(command)) {
            commandType_ = CommandType::C_LOGIC;
            arg1_ = command;
            toLower(arg1_);
        }
        else {
            throw std::runtime_error("Unknown/unsupported keyword '" + arg1_ + "'.\n");
        }
    }

    std::string Parser::arg1() const {
        return arg1_;
    }

    int Parser::arg2() const {
        return arg2_;
    }

    std::string Parser::line() const {
        return line_;
    }

    void Parser::toLower(std::string& s) const {
        std::ranges::transform(s, s.begin(), [](unsigned char c) {
            return std::tolower(c);
        });
    }

    bool Parser::isNumeric(const std::string& s) const {
        return !s.empty() && std::ranges::all_of(s, ::isdigit);
    }
}


