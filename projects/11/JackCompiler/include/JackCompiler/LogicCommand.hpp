#pragma once

#include <cstdint>
#include <string_view>

namespace JackCompiler {
    enum class LogicCommand : uint8_t {
        L_INVALID = 0,
        L_EQ,
        L_GT,
        L_LT,
        L_AND,
        L_OR,
        L_NOT
    };

    constexpr std::string_view LOGIC_COMMAND_STRINGS[] = {
        "invalid",
        "eq",
        "gt",
        "lt",
        "and",
        "or",
        "not"
    };

    inline constexpr std::string_view logicCommandToString(LogicCommand logicCommand) {
        return std::string_view(LOGIC_COMMAND_STRINGS[static_cast<int>(logicCommand)]);
    }

    inline constexpr bool isLogicCommand(std::string_view cmd) {
        for (std::string_view s : LOGIC_COMMAND_STRINGS) {
            if (s == cmd) return true;
        }
        return false;
    }

    inline constexpr LogicCommand parseLogicCommand(std::string_view cmd) {
        for (size_t i = 0; i < sizeof(LOGIC_COMMAND_STRINGS) / sizeof(LOGIC_COMMAND_STRINGS[0]); i++) {
            if (LOGIC_COMMAND_STRINGS[i] == cmd) {
                return static_cast<LogicCommand>(i);
            }
        }
        return LogicCommand::L_INVALID;
    }
}
