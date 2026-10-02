#include "JackAnalyzer/CompilationEngine.hpp"

#include <string>

#include "JackAnalyzer/JackTokenizer.hpp"
#include "JackAnalyzer/Keyword.hpp"
#include "JackAnalyzer/Symbol.hpp"
#include "JackAnalyzer/Token.hpp"

namespace fs = std::filesystem;

namespace JackAnalyzer {
    CompilationEngine::CompilationEngine(const fs::path& jackFilename) 
        : jackTokenizer_(jackFilename)
    {
        std::string outputFileName = fs::path(jackFilename).replace_extension(".xml");
        ofFile_.open(outputFileName);

        if (!ofFile_.is_open()) {
            throw std::runtime_error("Failed to open '" + outputFileName + "'.\n");
        }

        jackTokenizer_.advance();
    }

    void CompilationEngine::compileClass() {
        writeOpeningTag("class");
        compileKeyword(Keyword::Class);
        compileIdentifier();
        compileSymbol(Symbol::LBrace);
        compileClassVarDec();
        compileSubroutineDec();
        compileSymbol(Symbol::RBrace);
        writeClosingTag("class");

        if (jackTokenizer_.hasMoreTokens())
            throw std::runtime_error("Expected EOF.\n");
    }

    void CompilationEngine::compileClassVarDec() {
        Keyword currKeyword = isOneOfKeywords({Keyword::Static, Keyword::Field});
        if (currKeyword == Keyword::Invalid)
            return;

        writeOpeningTag("classVarDec");
        compileKeyword(currKeyword);
        compileType();
        compileIdentifier();
        compileVarDecExtension();
        compileSymbol(Symbol::Semicolon);
        writeClosingTag("classVarDec");
        compileClassVarDec();
    }

    void CompilationEngine::compileVarDecExtension(bool hasType) {
        Symbol currSymbol = isOneOfSymbols({Symbol::Comma});
        if (currSymbol == Symbol::Invalid)
            return;

        compileSymbol(Symbol::Comma);
        if (hasType) compileType();
        compileIdentifier();
        compileVarDecExtension(hasType);
    }

    void CompilationEngine::compileSubroutineDec() {
        Keyword currKeyword = isOneOfKeywords({Keyword::Constructor, Keyword::Function, Keyword::Method});
        if (currKeyword == Keyword::Invalid)
            return;

        writeOpeningTag("subroutineDec");
        compileKeyword(currKeyword);
        compileSubroutineType();
        compileIdentifier();
        compileSymbol(Symbol::LParen);
        compileParameterList();
        compileSymbol(Symbol::RParen);
        compileSubroutineBody();
        writeClosingTag("subroutineDec");
        compileSubroutineDec();
    }

    void CompilationEngine::compileSubroutineType() {
        if (jackTokenizer_.tokenType() == Token::KEYWORD 
            && jackTokenizer_.keyword() == Keyword::Void) {
            compileKeyword(Keyword::Void);
            return;
        }
        compileType();
    }

    void CompilationEngine::compileParameterList() {
        writeOpeningTag("parameterList");
        if (!isType()) {
            writeClosingTag("parameterList");
            return;
        }
        compileType();
        compileIdentifier();
        compileVarDecExtension(true);
        writeClosingTag("parameterList");
    }

    void CompilationEngine::compileSubroutineBody() {
        writeOpeningTag("subroutineBody");
        compileSymbol(Symbol::LBrace);
        compileVarDec();
        compileStatements();
        compileSymbol(Symbol::RBrace);
        writeClosingTag("subroutineBody");
    }

    void CompilationEngine::compileVarDec() {
        Keyword currKeyword = isOneOfKeywords({Keyword::Var});
        if (currKeyword == Keyword::Invalid)
            return;

        writeOpeningTag("varDec");
        compileKeyword(Keyword::Var);
        compileType();
        compileIdentifier();
        compileVarDecExtension();
        compileSymbol(Symbol::Semicolon);
        writeClosingTag("varDec");
        compileVarDec();
    }

    void CompilationEngine::compileStatements() {
        writeOpeningTag("statements");
        compileStatement();
        writeClosingTag("statements");
    }

    void CompilationEngine::compileStatement() {
        Keyword currKeyword = isOneOfKeywords({Keyword::Let, Keyword::If, Keyword::While, Keyword::Do, Keyword::Return});
        if (currKeyword == Keyword::Invalid)
            return;

        switch (currKeyword) {
            case Keyword::Let:    compileLet();    break;
            case Keyword::If:     compileIf();     break;
            case Keyword::While:  compileWhile();  break;
            case Keyword::Do:     compileDo();     break;
            case Keyword::Return: compileReturn(); break;
            default:                               break;
        }
        compileStatement();
    }

    void CompilationEngine::compileLet() {
        writeOpeningTag("letStatement");
        compileKeyword(Keyword::Let);
        compileIdentifier();
        compileArrayIndex();
        compileSymbol(Symbol::Equals);
        compileExpression();
        compileSymbol(Symbol::Semicolon);
        writeClosingTag("letStatement");
    }

    void CompilationEngine::compileIf() {
        writeOpeningTag("ifStatement");
        compileKeyword(Keyword::If);
        compileSymbol(Symbol::LParen);
        compileExpression();
        compileSymbol(Symbol::RParen);
        compileSymbol(Symbol::LBrace);
        compileStatements();
        compileSymbol(Symbol::RBrace);
        compileElse();
        writeClosingTag("ifStatement");
    }

    void CompilationEngine::compileElse() {
        Keyword currKeyword = isOneOfKeywords({Keyword::Else});
        if (currKeyword == Keyword::Invalid)
            return;

        compileKeyword(Keyword::Else);
        compileSymbol(Symbol::LBrace);
        compileStatements();
        compileSymbol(Symbol::RBrace);
    }

    void CompilationEngine::compileWhile() {
        writeOpeningTag("whileStatement");
        compileKeyword(Keyword::While);
        compileSymbol(Symbol::LParen);
        compileExpression();
        compileSymbol(Symbol::RParen);
        compileSymbol(Symbol::LBrace);
        compileStatements();
        compileSymbol(Symbol::RBrace);
        writeClosingTag("whileStatement");
    }

    void CompilationEngine::compileDo() {
        writeOpeningTag("doStatement");
        compileKeyword(Keyword::Do);
        compileSubroutineCall();
        compileSymbol(Symbol::Semicolon);
        writeClosingTag("doStatement");
    }

    void CompilationEngine::compileSubroutineCall(bool isIdentifierCompiled) {
        if (!isIdentifierCompiled)
            compileIdentifier();
        if (jackTokenizer_.tokenType() == Token::SYMBOL
            && jackTokenizer_.symbol() == Symbol::Dot) {
            compileSymbol(Symbol::Dot);
            compileIdentifier();
        }
        compileSymbol(Symbol::LParen);
        compileExpressionList();
        compileSymbol(Symbol::RParen);
    }

    void CompilationEngine::compileReturn() {
        writeOpeningTag("returnStatement");
        compileKeyword(Keyword::Return);
        if (isTerm())
            compileExpression();
        compileSymbol(Symbol::Semicolon);
        writeClosingTag("returnStatement");
    }

    void CompilationEngine::compileArrayIndex() {
        Symbol currSymbol = isOneOfSymbols({Symbol::LBracket});
        if (currSymbol == Symbol::Invalid)
            return;

        compileSymbol(Symbol::LBracket);
        compileExpression();
        compileSymbol(Symbol::RBracket);
    }

    void CompilationEngine::compileExpression() {
        writeOpeningTag("expression");
        compileTerm();
        compileTermExtension();
        writeClosingTag("expression");
    }

    void CompilationEngine::compileTerm() {
        writeOpeningTag("term");
        if (jackTokenizer_.tokenType() == Token::INT_CONST) {
            compileIntConst();
            writeClosingTag("term");
            return;
        }
        if (jackTokenizer_.tokenType() == Token::STRING_CONST) {
            compileStringConst();
            writeClosingTag("term");
            return;
        }
        Keyword currKeywordConst = isOneOfKeywordConstant();
        if (currKeywordConst != Keyword::Invalid) {
            compileKeyword(currKeywordConst);
            writeClosingTag("term");
            return;
        }
        // this includes: varName | varName[expr] | subroutineCall
        if (jackTokenizer_.tokenType() == Token::IDENTIFIER) {
            compileIdentifier();

            if (jackTokenizer_.tokenType() == Token::SYMBOL) {
                // check if possible varName[expr]
                if (jackTokenizer_.symbol() == Symbol::LBracket) {
                    compileArrayIndex();
                    writeClosingTag("term");
                    return;
                }

                // check if possible subroutineCall
                if (jackTokenizer_.symbol() == Symbol::LParen
                    || jackTokenizer_.symbol() == Symbol::Dot) {
                    compileSubroutineCall(true);
                    writeClosingTag("term");
                    return;
                }
            }

            writeClosingTag("term");
            return;
        }
        Symbol currSymbol = isOneOfSymbols({Symbol::LParen});
        if (currSymbol != Symbol::Invalid) {
            compileSymbol(Symbol::LParen);
            compileExpression();
            compileSymbol(Symbol::RParen);
            writeClosingTag("term");
            return;
        }
        Symbol currUnaryOp = isOneOfUnaryOp();
        if (currUnaryOp != Symbol::Invalid) {
            compileSymbol(currUnaryOp);
            compileTerm();
            writeClosingTag("term");
            return;
        }

        throw std::runtime_error("Expected term.\n");
    }

    void CompilationEngine::compileTermExtension() {
        Symbol currOp = isOneOfOp();
        if (currOp == Symbol::Invalid)
            return;

        compileSymbol(currOp);
        compileTerm();
        compileTermExtension();
    }

    void CompilationEngine::compileExpressionList() {
        writeOpeningTag("expressionList");
        if (!isTerm()) {
            writeClosingTag("expressionList");
            return;
        }
        compileExpression();
        compileExprListExtension();
        writeClosingTag("expressionList");
    }

    void CompilationEngine::compileExprListExtension() {
        Symbol currSymbol = isOneOfSymbols({Symbol::Comma});
        if (currSymbol == Symbol::Invalid)
            return;

        compileSymbol(Symbol::Comma);
        compileExpression();
        compileExprListExtension();
    }

    bool CompilationEngine::isType() {
        if (jackTokenizer_.tokenType() == Token::IDENTIFIER)
            return true;

        Keyword currKeyword = isOneOfKeywords({Keyword::Int, Keyword::Char, Keyword::Boolean});
        return currKeyword != Keyword::Invalid;
    }

    bool CompilationEngine::isTerm() {
        if (jackTokenizer_.tokenType() == Token::INT_CONST)
            return true;
        if (jackTokenizer_.tokenType() == Token::STRING_CONST)
            return true;
        if (isOneOfKeywordConstant() != Keyword::Invalid)
            return true;
        // this includes: varName | varName[expr] | subroutineCall
        if (jackTokenizer_.tokenType() == Token::IDENTIFIER)
            return true;
        if (isOneOfSymbols({Symbol::LParen}) != Symbol::Invalid)
            return true;
        if (isOneOfUnaryOp() != Symbol::Invalid)
            return true;
        return false;
    }

    Symbol CompilationEngine::isOneOfOp() {
        return isOneOfSymbols({Symbol::Plus, Symbol::Minus, Symbol::Multiply, Symbol::Divide, Symbol::Ampersand, Symbol::Pipe, Symbol::LessThan, Symbol::GreaterThan, Symbol::Equals});
    }
    Symbol CompilationEngine::isOneOfUnaryOp() {
        return isOneOfSymbols({Symbol::Minus, Symbol::Tilde});
    }
    Keyword CompilationEngine::isOneOfKeywordConstant() {
        return isOneOfKeywords({Keyword::True, Keyword::False, Keyword::Null, Keyword::This});
    }

    void CompilationEngine::compileKeyword(Keyword expected) {
        if (jackTokenizer_.tokenType() != Token::KEYWORD || 
            jackTokenizer_.keyword() != expected)
            throw std::runtime_error("Expected keyword: " + std::string(keywordToString(expected)));

        writeInlineTag("keyword", keywordToString(expected));
        jackTokenizer_.advance();
    }

    Keyword CompilationEngine::isOneOfKeywords(std::initializer_list<Keyword> expectedKeywords) {
        if (jackTokenizer_.tokenType() != Token::KEYWORD)
            return Keyword::Invalid;

        Keyword current = jackTokenizer_.keyword();
        
        for (Keyword expected : expectedKeywords) {
            if (current == expected) return current;
        }

        return Keyword::Invalid;
    }

    Symbol CompilationEngine::isOneOfSymbols(std::initializer_list<Symbol> expectedSymbols) {
        if (jackTokenizer_.tokenType() != Token::SYMBOL)
            return Symbol::Invalid;

        Symbol current = jackTokenizer_.symbol();
        
        for (Symbol expected : expectedSymbols) {
            if (current == expected) return current;
        }

        return Symbol::Invalid;
    }

    void CompilationEngine::compileIdentifier() {
        if (jackTokenizer_.tokenType() != Token::IDENTIFIER)
            throw std::runtime_error("Expected identifier.\n");

        writeInlineTag("identifier", jackTokenizer_.identifier());
        jackTokenizer_.advance();
    }

    void CompilationEngine::compileIntConst() {
        if (jackTokenizer_.tokenType() != Token::INT_CONST)
            throw std::runtime_error("Expected int constant.\n");

        writeInlineTag("integerConstant", std::to_string(jackTokenizer_.intVal()));
        jackTokenizer_.advance();
    }

    void CompilationEngine::compileStringConst() {
        if (jackTokenizer_.tokenType() != Token::STRING_CONST)
            throw std::runtime_error("Expected string constant.\n");

        writeInlineTag("stringConstant", jackTokenizer_.stringVal());
        jackTokenizer_.advance();
    }

    void CompilationEngine::compileSymbol(Symbol expected) {
        if (jackTokenizer_.tokenType() != Token::SYMBOL || 
            jackTokenizer_.symbol() != expected) {
            throw std::runtime_error("Expected symbol: " + std::string(symbolToString(expected)));
        }

        writeInlineTag("symbol", symbolToString(jackTokenizer_.symbol()));
        jackTokenizer_.advance();
    }

    void CompilationEngine::compileType() {
        if (jackTokenizer_.tokenType() == Token::IDENTIFIER) {
            compileIdentifier();
            return;
        }

        Keyword currKeyword = isOneOfKeywords({Keyword::Int, Keyword::Char, Keyword::Boolean});
        if (currKeyword == Keyword::Invalid) {
            throw std::runtime_error("Expected type.\n");
        }
        compileKeyword(currKeyword);
    }

    void CompilationEngine::writeIndentation() {
        for (uint8_t i = 0; i < currDepth_ * INDENT_SPACE_CNT; i++) {
            ofFile_ << ' ';
        }
    }

    void CompilationEngine::writeOpeningTag(std::string_view tag) {
        writeIndentation();
        ofFile_ << "<" << tag << ">\n";
        currDepth_++;
    }

    void CompilationEngine::writeClosingTag(std::string_view tag) {
        currDepth_--;
        writeIndentation();
        ofFile_ << "</" << tag << ">\n";
    }

    void CompilationEngine::writeInlineTag(std::string_view tag, std::string_view value) {
        writeIndentation();
        ofFile_ << "<" << tag << "> " << value << " </" << tag << ">\n";
    }
}
