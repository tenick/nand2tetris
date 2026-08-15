#include "VMTranslator/BranchingCommand.hpp"
#include "VMTranslator/CommandType.hpp"

#include <unordered_map>

namespace {
    static const std::unordered_map<std::string, VMTranslator::CommandType> stringToCommandCmdMap {
        {"label", VMTranslator::CommandType::C_LABEL},
        {"goto", VMTranslator::CommandType::C_GOTO},
        {"if-goto", VMTranslator::CommandType::C_IF},
    };
}

namespace VMTranslator {
    bool isStringABranchingCommand(const std::string& cmdText) {
        return stringToCommandCmdMap.find(cmdText) != stringToCommandCmdMap.end();
    }

    CommandType stringToBranchingCommand(const std::string &cmdText) {
        return stringToCommandCmdMap.at(cmdText);
    }
}
