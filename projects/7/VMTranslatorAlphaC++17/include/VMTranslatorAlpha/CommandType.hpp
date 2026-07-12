#pragma once

#include <cstdint>

namespace VMTranslator {
    enum class CommandType : uint8_t {
        C_ARITHMETIC,
        C_LOGIC,
        C_PUSH,
        C_POP,
        C_LABEL,
        C_GOTO,
        C_IF,
        C_FUNCTION,
        C_RETURN,
        C_CALL
    };
}
