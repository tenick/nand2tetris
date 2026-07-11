#include "VMTranslatorAlpha/LogicCommand.hpp"

#include <unordered_map>

namespace {
    static const std::unordered_map<std::string, VMTranslator::LogicCommand> stringToLogicCmdMap {
        {"eq", VMTranslator::LogicCommand::L_EQ},
        {"gt", VMTranslator::LogicCommand::L_GT},
        {"lt", VMTranslator::LogicCommand::L_LT},
        {"and", VMTranslator::LogicCommand::L_AND},
        {"or", VMTranslator::LogicCommand::L_OR},
        {"not", VMTranslator::LogicCommand::L_NOT}
    };
}

namespace VMTranslator {
    bool isStringALogicCommand(const std::string& cmdText) {
        return stringToLogicCmdMap.contains(cmdText);
    }

    VMTranslator::LogicCommand stringToLogicCommand(const std::string& cmdText) {
        return stringToLogicCmdMap.at(cmdText);
    }
}

