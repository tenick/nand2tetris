#pragma once

#include <cstdint>
#include <string_view>

namespace JackCompiler {
    enum class ArithmeticCommand : uint8_t {
        A_INVALID = 0,
        A_ADD,
        A_SUB,
        A_NEG
    };

    constexpr std::string_view ARITHMETIC_COMMAND_STRINGS[] = {
        "invalid",
        "add",
        "sub",
        "neg"
    };

    inline constexpr std::string_view arithmeticCommandToString(ArithmeticCommand arithmeticCommand) {
        return std::string_view(ARITHMETIC_COMMAND_STRINGS[static_cast<int>(arithmeticCommand)]);
    }

    inline constexpr bool isArithmeticCommand(std::string_view cmd) {
        for (std::string_view s : ARITHMETIC_COMMAND_STRINGS) {
            if (s == cmd) return true;
        }
        return false;
    }

    inline constexpr ArithmeticCommand parseArithmeticCommand(std::string_view cmd) {
        for (size_t i = 0; i < sizeof(ARITHMETIC_COMMAND_STRINGS) / sizeof(ARITHMETIC_COMMAND_STRINGS[0]); i++) {
            if (ARITHMETIC_COMMAND_STRINGS[i] == cmd) {
                return static_cast<ArithmeticCommand>(i);
            }
        }
        return ArithmeticCommand::A_INVALID;
    }
}
