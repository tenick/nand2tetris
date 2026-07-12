#pragma once

#include <cstdint>
#include <string>

namespace VMTranslator {
    enum class ArithmeticCommand : uint8_t {
        A_ADD,
        A_SUB,
        A_NEG
    };

    bool isStringAnArithmeticCommand(const std::string& cmdText);
    ArithmeticCommand stringToArithmeticCommand(const std::string& cmdText);
}
