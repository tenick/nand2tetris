#pragma once

#include <cstdint>

namespace JackCompiler {
    enum class Token : uint8_t {
        INVALID = 0,
        KEYWORD,
        SYMBOL,
        IDENTIFIER,
        INT_CONST,
        STRING_CONST
    };
}
