#include "VMTranslator/FunctionCommand.hpp"
#include "VMTranslator/CommandType.hpp"

#include <unordered_map>

namespace {
    static const std::unordered_map<std::string, VMTranslator::CommandType> stringToCommandCmdMap {
        {"call", VMTranslator::CommandType::C_CALL},
        {"function", VMTranslator::CommandType::C_FUNCTION},
        {"return", VMTranslator::CommandType::C_RETURN},
    };
}

namespace VMTranslator {
    bool isStringAFunctionCommand(const std::string& cmdText) {
        return stringToCommandCmdMap.find(cmdText) != stringToCommandCmdMap.end();
    }

    CommandType stringToFunctionCommand(const std::string &cmdText) {
        return stringToCommandCmdMap.at(cmdText);
    }
}

