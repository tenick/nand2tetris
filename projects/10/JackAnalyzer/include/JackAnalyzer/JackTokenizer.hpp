#pragma once

#include <fstream>
#include <filesystem>

#include "JackAnalyzer/Token.hpp"
#include "JackAnalyzer/Keyword.hpp"
#include "JackAnalyzer/Symbol.hpp"

namespace fs = std::filesystem;

namespace JackAnalyzer {
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

    private:
        std::ifstream inFile_;

        bool hasMoreTokens_ = true;
        Token currentToken_ = Token::INVALID;
        Keyword currentKeyword_ = Keyword::Invalid;
        Symbol currentSymbol_ = Symbol::Invalid;
        int currentIntConst_ = -1;
        std::string currentTokenStr_;

        static constexpr int JACK_MAX_INT = 32767;

        bool isWhitespace(char) const noexcept;
        // if currently at a whitespace, go through all whitespaces first, until it reach non-whitespace
        void skipWhitespaces();
        bool isSymbol(char ch) const;
        bool isAlNum(char ch) const;
        bool isIdentifierChar(char ch) const;
    };
}
