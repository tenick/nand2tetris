#pragma once

#include <cstdint>
#include <fstream>

#include "JackCompiler/JackTokenizer.hpp"
#include "JackCompiler/Keyword.hpp"
#include "JackCompiler/SymbolTable.hpp"
#include "JackCompiler/VMWriter.hpp"

namespace fs = std::filesystem;

namespace JackCompiler {
    class CompilationEngine {
    public:
        CompilationEngine(const fs::path&);
        void compileClass();
        void compileClassVarDec();
        void compileSubroutineDec();
        void compileParameterList();
        void compileSubroutineBody(std::string_view, Keyword);
        int compileVarDec();
        void compileStatements();

        void compileLet();
        void compileIf();
        void compileWhile();
        void compileDo();
        void compileReturn();

        void compileExpression();
        void compileTerm();
        int compileExpressionList();

    private:
        std::ofstream ofFile_;
        JackTokenizer jackTokenizer_;
        SymbolTable symbolTable_;
        VMWriter vmWriter_;
        static uint8_t constexpr INDENT_SPACE_CNT = 2;
        uint8_t currDepth_ = 0;
        uint8_t currLblCnt_ = 0;
        const std::string CLASS_NAME;
        const std::string MATH_MULTIPLY_CALL = "Math.multiply";
        const std::string MATH_DIVIDE_CALL = "Math.divide";
        const std::string MEMORY_ALLOC_CALL = "Memory.alloc";
        const std::string STRING_NEW_CALL = "String.new";
        const std::string STRING_APPEND_CALL = "String.appendChar";

        std::string compileKeyword(Keyword);
        Keyword isOneOfKeywords(std::initializer_list<Keyword>);
        Symbol isOneOfSymbols(std::initializer_list<Symbol>);
        bool isType();
        std::string compileIdentifier();
        Symbol compileSymbol(Symbol);
        void compileIntConst();
        void compileStringConst();
        std::string compileType();
        void compileSubroutineType();
        int compileVarDecExtension(SymbolKind, std::string&, bool=false);
        bool compileArrayIndex(const std::string&, bool=true);
        void compileStatement();
        void compileElse();
        void compileSubroutineCall(bool=false, std::string="");
        void compileTermExtension();
        int compileExprListExtension();
        bool isTerm();
        Symbol isOneOfOp();
        Symbol isOneOfUnaryOp();
        Keyword isOneOfKeywordConstant();

        void writeIndentation();
        void writeOpeningTag(std::string_view);
        void writeClosingTag(std::string_view);
        void writeInlineTag(std::string_view, std::string_view);

        void symbolTableDefine(const std::string& name, const std::string& type, SymbolKind kind);;
    };
}
