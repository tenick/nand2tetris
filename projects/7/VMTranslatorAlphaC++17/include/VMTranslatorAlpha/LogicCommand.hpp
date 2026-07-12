#pragma once

#include <cstdint>
#include <string>

namespace VMTranslator {
    enum class LogicCommand : uint8_t {
        L_EQ,
        L_GT,
        L_LT,
        L_AND,
        L_OR,
        L_NOT
    };

    bool isStringALogicCommand(const std::string& cmdText);
    LogicCommand stringToLogicCommand(const std::string& cmdText);
}
