#pragma once

#include <cstdint>
#include <endian.h>
#include <string_view>

namespace JackAnalyzer {
    enum class Keyword : uint8_t {
        Invalid = 0,
        Class,
        Method,
        Function,
        Constructor,
        Int,
        Boolean,
        Char,
        Void,
        Var,
        Static,
        Field,
        Let,
        Do,
        If,
        Else,
        While,
        Return,
        True,
        False,
        Null,
        This
    };

    constexpr std::string_view KEYWORD_STRINGS[] = {
        "invalid",
        "class",
        "method",
        "function",
        "constructor",
        "int",
        "boolean",
        "char",
        "void",
        "var",
        "static",
        "field",
        "let",
        "do",
        "if",
        "else",
        "while",
        "return",
        "true",
        "false",
        "null",
        "this"
    };

    inline constexpr std::string_view keywordToString(Keyword keyword) {
        return std::string_view(KEYWORD_STRINGS[static_cast<int>(keyword)]);
    }
}
