#pragma once

#include <fstream>
#include <filesystem>

#include "JackCompiler/Token.hpp"
#include "JackCompiler/Keyword.hpp"
#include "JackCompiler/Symbol.hpp"

namespace fs = std::filesystem;

namespace JackCompiler {
    class JackTokenizer {
    public:
        JackTokenizer(const fs::path&);
        bool hasMoreTokens() const noexcept;
        void advance();
        Token tokenType();

        Keyword keyword() const;
        Symbol symbol() const;
        std::string identifier() const;
        int intVal() const;
        std::string stringVal() const;

        bool nextChar(char&);
        int currentLine() const;
        int currentLineColumn() const;
        std::string currentCursorLocation() const;

    private:
        std::ifstream inFile_;

        bool hasMoreTokens_ = true;
        Token currentToken_ = Token::INVALID;
        Keyword currentKeyword_ = Keyword::Invalid;
        Symbol currentSymbol_ = Symbol::Invalid;
        int currentIntConst_ = -1;
        std::string currentTokenStr_;

        int currLine_{0};
        int currLineColumn_{0};

        static constexpr int JACK_MAX_INT = 32767;

        bool isWhitespace(char) const noexcept;
        // if currently at a whitespace, go through all whitespaces first, until it reach non-whitespace
        void skipWhitespaces();
        bool isSymbol(char ch) const;
        bool isAlNum(char ch) const;
        bool isIdentifierChar(char ch) const;
    };
}
