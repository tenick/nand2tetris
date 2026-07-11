#include "VMTranslatorAlpha/ArithmeticCommand.hpp"

#include <unordered_map>

namespace {
    static const std::unordered_map<std::string, VMTranslator::ArithmeticCommand> stringToArithmeticCmdMap {
        {"add", VMTranslator::ArithmeticCommand::A_ADD},
        {"sub", VMTranslator::ArithmeticCommand::A_SUB},
        {"neg", VMTranslator::ArithmeticCommand::A_NEG}
    };
}

namespace VMTranslator {
    bool isStringAnArithmeticCommand(const std::string& cmdText) {
        return stringToArithmeticCmdMap.contains(cmdText);
    }

    VMTranslator::ArithmeticCommand stringToArithmeticCommand(const std::string& cmdText) {
        return stringToArithmeticCmdMap.at(cmdText);
    }
}
