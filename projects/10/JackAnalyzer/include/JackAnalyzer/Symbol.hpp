#pragma once

#include <cstdint>
#include <string_view>

namespace JackAnalyzer {
    enum class Symbol : uint8_t {
        Invalid = 0,
        LBrace, RBrace,       // { }
        LParen, RParen,       // ( )
        LBracket, RBracket,   // [ ]
        Dot, Comma, Semicolon,// . , ;
        Plus, Minus,          // + -
        Multiply, Divide,     // * /
        Ampersand, Pipe,      // & |
        LessThan,             // <
        GreaterThan,          // >
        Equals,               // =
        Tilde                 // ~
    };

    inline static const char SYMBOL_STRINGS[] = " {}()[].,;+-*/&|<>=~";

    inline constexpr std::string_view symbolToString(Symbol symbol) {
        switch (symbol) {
            case Symbol::LessThan:      return "&lt;";
            case Symbol::GreaterThan:   return "&gt;";
            case Symbol::Ampersand:     return "&amp;";
            default:
                return std::string_view(&SYMBOL_STRINGS[static_cast<int>(symbol)], 1);
        }
        return "";
    }
}
