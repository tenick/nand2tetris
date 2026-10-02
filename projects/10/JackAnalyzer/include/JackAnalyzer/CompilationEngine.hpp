#pragma once

#include <cstdint>
#include <fstream>

#include "JackAnalyzer/JackTokenizer.hpp"
#include "JackAnalyzer/Keyword.hpp"

namespace fs = std::filesystem;

namespace JackAnalyzer {
    class CompilationEngine {
    public:
        CompilationEngine(const fs::path&);
        void compileClass();
        void compileClassVarDec();
        void compileSubroutineDec();
        void compileParameterList();
        void compileSubroutineBody();
        void compileVarDec();
        void compileStatements();

        void compileLet();
        void compileIf();
        void compileWhile();
        void compileDo();
        void compileReturn();

        void compileExpression();
        void compileTerm();
        void compileExpressionList();

    private:
        std::ofstream ofFile_;
        JackTokenizer jackTokenizer_;
        static uint8_t constexpr INDENT_SPACE_CNT = 2;
        uint8_t currDepth_ = 0;

        void compileKeyword(Keyword);
        Keyword isOneOfKeywords(std::initializer_list<Keyword>);
        Symbol isOneOfSymbols(std::initializer_list<Symbol>);
        bool isType();
        void compileIdentifier();
        void compileSymbol(Symbol);
        void compileIntConst();
        void compileStringConst();
        void compileType();
        void compileSubroutineType();
        void compileVarDecExtension(bool=false);
        void compileArrayIndex();
        void compileStatement();
        void compileElse();
        void compileSubroutineCall(bool=false);
        void compileTermExtension();
        void compileExprListExtension();
        bool isTerm();
        Symbol isOneOfOp();
        Symbol isOneOfUnaryOp();
        Keyword isOneOfKeywordConstant();

        void writeIndentation();
        void writeOpeningTag(std::string_view);
        void writeClosingTag(std::string_view);
        void writeInlineTag(std::string_view, std::string_view);
    };
}
