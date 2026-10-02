#include "JackAnalyzer/JackTokenizer.hpp"

#include <cctype>
#include <cstdio>
#include <string>
#include <cstring>

#include "JackAnalyzer/Keyword.hpp"
#include "JackAnalyzer/Token.hpp"
#include "JackAnalyzer/Symbol.hpp"

namespace JackAnalyzer {
    JackTokenizer::JackTokenizer(const fs::path& filepath) 
        : inFile_(filepath)
    {
        if (!inFile_.is_open()) {
            throw std::runtime_error("Failed to open '" + filepath.string() + "'.\n");
        }
        if (inFile_.peek() == EOF) {
            throw std::runtime_error("'" + filepath.string() + " is an empty file.\n");
        }

        skipWhitespaces();

        if (inFile_.eof()) {
            hasMoreTokens_ = false;
        }
    }

    bool JackTokenizer::hasMoreTokens() const noexcept {
        return hasMoreTokens_;
    }

    Token JackTokenizer::tokenType() {
        return currentToken_;
    }

    void JackTokenizer::advance() {
        currentTokenStr_.clear();
        currentToken_ = Token::INVALID;
        currentKeyword_ = Keyword::Invalid;
        currentSymbol_ = Symbol::Invalid;
        currentIntConst_ = -1;

        if (inFile_.eof()) {
            return;
        }

        // invariant: guaranteed file.get()/.peek() always have non-whitespace and non-EOF here
        char currCh;
        inFile_.get(currCh);

        // check if it's a comment first
        if (currCh == '/') {
            char nextCh = inFile_.peek();
            if (nextCh == '/') {
                // consume the '/' char first
                char ch;
                inFile_.get(ch);

                // keep looping until u reach line ending or hit EOF
                while (inFile_.get(ch)) {
                    if (ch == '\n') {
                        // found line ending
                        skipWhitespaces();
                        advance();
                        break;
                    }
                }
            }
            else if (nextCh == '*') {
                // consume the '*' char first
                char ch;
                inFile_.get(ch);

                // keep looping until u find "*/" or hit EOF
                while (inFile_.get(ch)) {
                    if (ch == '*' && inFile_.peek() == '/') {
                        // found comment ending
                        // consume '/'
                        inFile_.get(ch);
                        skipWhitespaces();
                        advance();
                        break;
                    }
                }
            }
            else {
                currentToken_ = Token::SYMBOL;
                currentSymbol_ = Symbol::Divide;
                currentTokenStr_.push_back(currCh);
                skipWhitespaces();
            }

            if (inFile_.eof()) hasMoreTokens_ = false;

            return;
        }

        // check if it's an int constant
        if (std::isdigit(currCh)) {
            currentToken_ = Token::INT_CONST;

            int currDigit = currCh - '0';
            currentTokenStr_.push_back(currCh);

            // try to keep getting next int, until you either:
            // a. reach a non-int character
            // b. the integer becomes larger than JACK_MAX_INT
            while (std::isdigit(inFile_.peek())) {
                char ch = inFile_.peek();
                int newDigit = currDigit*10 + ch - '0';
                if (newDigit > JACK_MAX_INT)
                    break;
                currDigit = newDigit;
                inFile_.get(ch);
                currentTokenStr_.push_back(ch);
            }
            currentIntConst_ = currDigit;

            skipWhitespaces();

            if (inFile_.eof()) hasMoreTokens_ = false;

            return;
        }

        // check if it's a string constant
        if (currCh == '"') {
            currentToken_ = Token::STRING_CONST;
            char ch;
            while (inFile_.get(ch)) {
                if (ch == '\n') {
                    // reach end of line before end of string, throw error
                    throw std::runtime_error("Failed to find end of string.\n");
                }
                if (ch == '"') {
                    // reached end of string
                    break;
                }
                currentTokenStr_.push_back(ch);
            }

            skipWhitespaces();

            if (inFile_.eof()) hasMoreTokens_ = false;

            return;
        }

        // check if it's a symbol
        if (isSymbol(currCh)) {
            currentToken_ = Token::SYMBOL;
            currentSymbol_ = static_cast<Symbol>(std::string_view(SYMBOL_STRINGS).find(currCh));
            currentTokenStr_.push_back(currCh);

            skipWhitespaces();

            if (inFile_.eof()) hasMoreTokens_ = false;

            return;
        }

        // check if it's a possible identifier
        if (isIdentifierChar(currCh)) {
            currentToken_ = Token::IDENTIFIER;
            currentTokenStr_.push_back(currCh);

            // then just keep looping until it reaches a non-alphanumeric and non-underscore
            char ch;
            while(isIdentifierChar(inFile_.peek())) {
                inFile_.get(ch);
                currentTokenStr_.push_back(ch);
            }

            // then check if final string formed is a possible keyword
            for (size_t i = 1; i < std::size(KEYWORD_STRINGS); i++) {
                if (KEYWORD_STRINGS[i] == currentTokenStr_) {
                    currentToken_ = Token::KEYWORD;
                    currentKeyword_ = static_cast<Keyword>(i);
                    break;
                }
            }

            skipWhitespaces();

            if (inFile_.eof()) hasMoreTokens_ = false;

            return;
        }
        
        // if it's not any of the above, just throw an error
        throw std::runtime_error(std::string("Unknown character '") + currCh + "'.\n");
    }

    Keyword JackTokenizer::keyword() const {
        if (currentToken_ != Token::KEYWORD)
            throw std::runtime_error("Current token is not a keyword\n");

        return currentKeyword_;
    }

    Symbol JackTokenizer::symbol() const {
        if (currentToken_ != Token::SYMBOL)
            throw std::runtime_error("Current token is not a symbol\n");

        return currentSymbol_;
    }

    std::string JackTokenizer::identifier() const {
        if (currentToken_ != Token::IDENTIFIER)
            throw std::runtime_error("Current token is not an identifier\n");

        return currentTokenStr_;
    }

    int JackTokenizer::intVal() const {
        if (currentToken_ != Token::INT_CONST)
            throw std::runtime_error("Current token is not an int const\n");

        return currentIntConst_;
    }

    std::string JackTokenizer::stringVal() const {
        if (currentToken_ != Token::STRING_CONST)
            throw std::runtime_error("Current token is not a string const\n");

        return currentTokenStr_;
    }

    bool JackTokenizer::isWhitespace(char ch) const noexcept {
        return std::isspace(static_cast<unsigned char>(ch));
    }

    void JackTokenizer::skipWhitespaces() {
        char ch;
        while (isWhitespace(inFile_.peek())) {
            inFile_.get(ch);
        }
    }

    bool JackTokenizer::isSymbol(char ch) const {
        return std::strchr(SYMBOL_STRINGS, ch) != nullptr;
    }

    bool JackTokenizer::isAlNum(char ch) const {
        return std::isalnum(static_cast<unsigned char>(ch));
    }

    bool JackTokenizer::isIdentifierChar(char ch) const {
        return isAlNum(ch) || ch == '_';
    }
}
