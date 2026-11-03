#include "JackCompiler/CompilationEngine.hpp"

#include <exception>
#include <stdexcept>
#include <string>
#include <string_view>

#include "JackCompiler/JackTokenizer.hpp"
#include "JackCompiler/Keyword.hpp"
#include "JackCompiler/LogicCommand.hpp"
#include "JackCompiler/MemorySegment.hpp"
#include "JackCompiler/Symbol.hpp"
#include "JackCompiler/SymbolKind.hpp"
#include "JackCompiler/SymbolTable.hpp"
#include "JackCompiler/Token.hpp"
#include "JackCompiler/VMWriter.hpp"

namespace fs = std::filesystem;

namespace JackCompiler {
    CompilationEngine::CompilationEngine(const fs::path& jackFilename) 
        : jackTokenizer_(jackFilename), vmWriter_(jackFilename), CLASS_NAME(jackFilename.stem()) {}

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

        auto kind = currKeyword == Keyword::Static ? SymbolKind::Static : SymbolKind::Field;

        writeOpeningTag("classVarDec");
        compileKeyword(currKeyword);
        auto type = compileType();
        auto identifier = compileIdentifier();
        symbolTableDefine(identifier, type, kind);
        compileVarDecExtension(kind, type);
        compileSymbol(Symbol::Semicolon);
        writeClosingTag("classVarDec");
        compileClassVarDec();
    }

    int CompilationEngine::compileVarDecExtension(SymbolKind kind, std::string& type, bool hasType) {
        Symbol currSymbol = isOneOfSymbols({Symbol::Comma});
        if (currSymbol == Symbol::Invalid)
            return 0;

        compileSymbol(Symbol::Comma);
        if (hasType) type = compileType();
        auto identifier = compileIdentifier();
        symbolTable_.define(identifier, type, kind);
        return 1 + compileVarDecExtension(kind, type, hasType);
    }

    void CompilationEngine::compileSubroutineDec() {
        Keyword currKeyword = isOneOfKeywords({Keyword::Constructor, Keyword::Function, Keyword::Method});
        if (currKeyword == Keyword::Invalid)
            return;

        writeOpeningTag("subroutineDec");
        symbolTable_.startSubroutine();
        compileKeyword(currKeyword);
        compileSubroutineType();
        auto subroutineIdentifier = compileIdentifier();
        compileSymbol(Symbol::LParen);
        if (currKeyword == Keyword::Method) {
            symbolTableDefine("dummyVarToMoveArgumentIndex", "dummyType", SymbolKind::Arg);
        }
        compileParameterList();
        compileSymbol(Symbol::RParen);
        compileSubroutineBody(subroutineIdentifier, currKeyword);
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
        auto type = compileType();
        auto identifier = compileIdentifier();
        symbolTableDefine(identifier, type, SymbolKind::Arg);
        compileVarDecExtension(SymbolKind::Arg, type, true);
        writeClosingTag("parameterList");
    }

    void CompilationEngine::compileSubroutineBody(std::string_view subroutineIdentifier, Keyword subroutineKind) {
        writeOpeningTag("subroutineBody");
        compileSymbol(Symbol::LBrace);
        int localVarCnt = compileVarDec();
        vmWriter_.writeFunction(CLASS_NAME + "." + std::string(subroutineIdentifier), localVarCnt);
        switch (subroutineKind) {
            case Keyword::Constructor: 
                vmWriter_.writePush(MemorySegment::M_CONSTANT, symbolTable_.varCount(SymbolKind::Field));
                vmWriter_.writeCall(MEMORY_ALLOC_CALL, 1);
                vmWriter_.writePop(MemorySegment::M_POINTER, 0);
                break;
            case Keyword::Method:
                vmWriter_.writePush(MemorySegment::M_ARGUMENT, 0);
                vmWriter_.writePop(MemorySegment::M_POINTER, 0);
                break;
            case Keyword::Function: break;
            default: throw std::runtime_error("Invalid subroutine kind '" + std::string(keywordToString(subroutineKind)) + "'.\n");
        }
        compileStatements();
        compileSymbol(Symbol::RBrace);
        writeClosingTag("subroutineBody");
    }

    int CompilationEngine::compileVarDec() {
        Keyword currKeyword = isOneOfKeywords({Keyword::Var});
        if (currKeyword == Keyword::Invalid)
            return 0;

        writeOpeningTag("varDec");
        compileKeyword(Keyword::Var);
        auto type = compileType();
        auto identifier = compileIdentifier();
        symbolTable_.define(identifier, type, SymbolKind::Var);
        int varDecExtCnt = compileVarDecExtension(SymbolKind::Var, type);
        compileSymbol(Symbol::Semicolon);
        writeClosingTag("varDec");
        return 1 + varDecExtCnt + compileVarDec();
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
        auto identifier = compileIdentifier();
        bool isArrAssignment = compileArrayIndex(identifier, false);
        compileSymbol(Symbol::Equals);
        compileExpression();
        if (isArrAssignment) {
            vmWriter_.writePop(MemorySegment::M_TEMP, 0);
            vmWriter_.writePop(MemorySegment::M_POINTER, 1);
            vmWriter_.writePush(MemorySegment::M_TEMP, 0);
            vmWriter_.writePop(MemorySegment::M_THAT, 0);
        }
        else {
            vmWriter_.writePop(symbolKindToMemorySegment(symbolTable_.kindOf(identifier)), symbolTable_.indexOf(identifier));
        }
        compileSymbol(Symbol::Semicolon);
        writeClosingTag("letStatement");
    }

    void CompilationEngine::compileIf() {
        writeOpeningTag("ifStatement");
        compileKeyword(Keyword::If);
        compileSymbol(Symbol::LParen);
        compileExpression();
        vmWriter_.writeLogic(LogicCommand::L_NOT);
        int currLblCntLocal = currLblCnt_;
        currLblCnt_ += 2;
        vmWriter_.writeIf(CLASS_NAME + "_if_Label_" + std::to_string(currLblCntLocal));
        compileSymbol(Symbol::RParen);
        compileSymbol(Symbol::LBrace);
        compileStatements();
        vmWriter_.writeGoto(CLASS_NAME + "_if_Label_" + std::to_string(currLblCntLocal+1));
        compileSymbol(Symbol::RBrace);
        vmWriter_.writeLabel(CLASS_NAME + "_if_Label_" + std::to_string(currLblCntLocal));
        compileElse();
        vmWriter_.writeLabel(CLASS_NAME + "_if_Label_" + std::to_string(currLblCntLocal+1));
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
        int currLblCntLocal = currLblCnt_;
        currLblCnt_ += 2;
        vmWriter_.writeLabel(CLASS_NAME + "_while_Label_" + std::to_string(currLblCntLocal));
        compileExpression();
        vmWriter_.writeLogic(LogicCommand::L_NOT);
        vmWriter_.writeIf(CLASS_NAME + "_while_Label_" + std::to_string(currLblCntLocal+1));
        compileSymbol(Symbol::RParen);
        compileSymbol(Symbol::LBrace);
        compileStatements();
        vmWriter_.writeGoto(CLASS_NAME + "_while_Label_" + std::to_string(currLblCntLocal));
        vmWriter_.writeLabel(CLASS_NAME + "_while_Label_" + std::to_string(currLblCntLocal+1));
        compileSymbol(Symbol::RBrace);
        writeClosingTag("whileStatement");
    }

    void CompilationEngine::compileDo() {
        writeOpeningTag("doStatement");
        compileKeyword(Keyword::Do);
        compileSubroutineCall();
        vmWriter_.writePop(MemorySegment::M_TEMP, 0);
        compileSymbol(Symbol::Semicolon);
        writeClosingTag("doStatement");
    }

    void CompilationEngine::compileSubroutineCall(bool isIdentifierCompiled, std::string identifier) {
        if (!isIdentifierCompiled)
            identifier = compileIdentifier();
        std::string subroutineCall;
        int addObj = 0;
        if (jackTokenizer_.tokenType() == Token::SYMBOL
            && (jackTokenizer_.symbol() == Symbol::LParen
                || jackTokenizer_.symbol() == Symbol::Dot)) {
            if (jackTokenizer_.symbol() == Symbol::LParen) {
                // method call inside current class
                // contract is when you enter a constructor/method subroutine,`this` is set to current object, so push pointer 0
                vmWriter_.writePush(MemorySegment::M_POINTER, 0);
                addObj++;
                subroutineCall = CLASS_NAME + "." + identifier;
            }
            else /* it's a dot */{
                compileSymbol(Symbol::Dot);
                if (symbolTable_.exists(identifier)) {
                    // object method call
                    vmWriter_.writePush(symbolKindToMemorySegment(symbolTable_.kindOf(identifier)), symbolTable_.indexOf(identifier));
                    addObj++;
                    subroutineCall = symbolTable_.typeOf(identifier);
                }
                else {
                    // class function call / constructor call (no-op)
                    subroutineCall = identifier;
                }
                subroutineCall += "." + compileIdentifier();
            }
        }
        compileSymbol(Symbol::LParen);
        int nArgs = compileExpressionList();
        compileSymbol(Symbol::RParen);
        vmWriter_.writeCall(subroutineCall, nArgs+addObj);
    }

    void CompilationEngine::compileReturn() {
        writeOpeningTag("returnStatement");
        compileKeyword(Keyword::Return);
        if (isTerm())
            compileExpression();
        else 
            vmWriter_.writePush(MemorySegment::M_CONSTANT, 0);

        vmWriter_.writeReturn();
        compileSymbol(Symbol::Semicolon);
        writeClosingTag("returnStatement");
    }

    bool CompilationEngine::compileArrayIndex(const std::string& identifier, bool evalArrAccess) {
        Symbol currSymbol = isOneOfSymbols({Symbol::LBracket});
        if (currSymbol == Symbol::Invalid)
            return false;

        compileSymbol(Symbol::LBracket);
        vmWriter_.writePush(symbolKindToMemorySegment(symbolTable_.kindOf(identifier)), symbolTable_.indexOf(identifier));
        compileExpression();
        vmWriter_.writeArithmetic(ArithmeticCommand::A_ADD);
        if (evalArrAccess) {
            vmWriter_.writePop(MemorySegment::M_POINTER, 1);
            vmWriter_.writePush(MemorySegment::M_THAT, 0);
        }
        compileSymbol(Symbol::RBracket);
        return true;
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
            auto identifier = compileIdentifier();

            if (jackTokenizer_.tokenType() == Token::SYMBOL) {
                // check if varName[expr]
                if (jackTokenizer_.symbol() == Symbol::LBracket) {
                    compileArrayIndex(identifier);
                    writeClosingTag("term");
                    return;
                }

                // check if subroutineCall
                if (jackTokenizer_.symbol() == Symbol::LParen
                    || jackTokenizer_.symbol() == Symbol::Dot) {
                    compileSubroutineCall(true, identifier);
                    writeClosingTag("term");
                    return;
                }
            }

            auto kind = symbolTable_.kindOf(identifier);
            switch (kind) {
                case SymbolKind::Var:
                    vmWriter_.writePush(MemorySegment::M_LOCAL, symbolTable_.indexOf(identifier));
                    break;
                case SymbolKind::Arg:
                    vmWriter_.writePush(MemorySegment::M_ARGUMENT, symbolTable_.indexOf(identifier));
                    break;
                case SymbolKind::Static:
                    vmWriter_.writePush(MemorySegment::M_STATIC, symbolTable_.indexOf(identifier));
                    break;
                case SymbolKind::Field:
                    vmWriter_.writePush(MemorySegment::M_THIS, symbolTable_.indexOf(identifier));
                    break;
                default: /* intentionally no-op */ break;
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
            auto op = compileSymbol(currUnaryOp);
            compileTerm();
            switch (op) {
                case Symbol::Minus:
                    vmWriter_.writeArithmetic(ArithmeticCommand::A_NEG);
                    break;
                case Symbol::Tilde:
                    vmWriter_.writeLogic(LogicCommand::L_NOT);
                    break;
                default: throw std::runtime_error(jackTokenizer_.currentCursorLocation() + " | Unexpected unary operation: " + std::string(symbolToString(op)));
            }
            writeClosingTag("term");
            return;
        }

        throw std::runtime_error("Expected term.\n");
    }

    void CompilationEngine::compileTermExtension() {
        Symbol currOp = isOneOfOp();
        if (currOp == Symbol::Invalid)
            return;

        auto op = compileSymbol(currOp);
        compileTerm();
        switch (op) {
            case Symbol::Plus:
                vmWriter_.writeArithmetic(ArithmeticCommand::A_ADD);
                break;
            case Symbol::Minus:
                vmWriter_.writeArithmetic(ArithmeticCommand::A_SUB);
                break;
            case Symbol::Multiply:
                vmWriter_.writeCall(MATH_MULTIPLY_CALL, 2);
                break;
            case Symbol::Divide:
                vmWriter_.writeCall(MATH_DIVIDE_CALL, 2);
                break;
            case Symbol::Ampersand:
                vmWriter_.writeLogic(LogicCommand::L_AND);
                break;
            case Symbol::Pipe:
                vmWriter_.writeLogic(LogicCommand::L_OR);
                break;
            case Symbol::LessThan:
                vmWriter_.writeLogic(LogicCommand::L_LT);
                break;
            case Symbol::GreaterThan:
                vmWriter_.writeLogic(LogicCommand::L_GT);
                break;
            case Symbol::Equals:
                vmWriter_.writeLogic(LogicCommand::L_EQ);
                break;
            default: throw std::runtime_error("Unexpected operation: " + std::string(symbolToString(op)));
        }
        compileTermExtension();
    }

    int CompilationEngine::compileExpressionList() {
        writeOpeningTag("expressionList");
        if (!isTerm()) {
            writeClosingTag("expressionList");
            return 0;
        }
        compileExpression();
        int cnt = compileExprListExtension();
        writeClosingTag("expressionList");
        return cnt + 1;
    }

    int CompilationEngine::compileExprListExtension() {
        Symbol currSymbol = isOneOfSymbols({Symbol::Comma});
        if (currSymbol == Symbol::Invalid)
            return 0;

        compileSymbol(Symbol::Comma);
        compileExpression();
        return 1 + compileExprListExtension();
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

    std::string CompilationEngine::compileKeyword(Keyword expected) {
        if (jackTokenizer_.tokenType() != Token::KEYWORD || 
            jackTokenizer_.keyword() != expected)
            throw std::runtime_error(jackTokenizer_.currentCursorLocation() + " | Expected keyword: " + std::string(keywordToString(expected)));

        // keyword constants handling
        switch (expected) {
            case Keyword::True:
                vmWriter_.writePush(MemorySegment::M_CONSTANT, 1);
                vmWriter_.writeArithmetic(ArithmeticCommand::A_NEG);
                break;
            case Keyword::False:
            case Keyword::Null:
                vmWriter_.writePush(MemorySegment::M_CONSTANT, 0);
                break;
            case Keyword::This:
                vmWriter_.writePush(MemorySegment::M_POINTER, 0);
                break;
            default: /* intentionally no-op */ break;
        }

        auto keyword = keywordToString(expected);
        writeInlineTag("keyword", keyword);
        jackTokenizer_.advance();
        return std::string(keyword);
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

    std::string CompilationEngine::compileIdentifier() {
        if (jackTokenizer_.tokenType() != Token::IDENTIFIER)
            throw std::runtime_error("Expected identifier.\n");

        std::string identifier = jackTokenizer_.identifier();
        writeInlineTag("identifier", identifier);
        jackTokenizer_.advance();

        return identifier;
    }

    void CompilationEngine::compileIntConst() {
        if (jackTokenizer_.tokenType() != Token::INT_CONST)
            throw std::runtime_error("Expected int constant.\n");

        writeInlineTag("integerConstant", std::to_string(jackTokenizer_.intVal()));
        vmWriter_.writePush(MemorySegment::M_CONSTANT, jackTokenizer_.intVal());
        jackTokenizer_.advance();
    }

    void CompilationEngine::compileStringConst() {
        if (jackTokenizer_.tokenType() != Token::STRING_CONST)
            throw std::runtime_error("Expected string constant.\n");

        auto strConstVal = jackTokenizer_.stringVal();
        writeInlineTag("stringConstant", strConstVal);
        vmWriter_.writePush(MemorySegment::M_CONSTANT, strConstVal.size());
        vmWriter_.writeCall(STRING_NEW_CALL, 1);
        for (char c : strConstVal) {
            // Since ASCII 32-126 matches 1:1, casting to unsigned char/int just works
            vmWriter_.writePush(MemorySegment::M_CONSTANT, static_cast<int>(static_cast<unsigned char>(c)));
            vmWriter_.writeCall(STRING_APPEND_CALL, 2);
        }
        jackTokenizer_.advance();
    }

    Symbol CompilationEngine::compileSymbol(Symbol expected) {
        if (jackTokenizer_.tokenType() != Token::SYMBOL || 
            jackTokenizer_.symbol() != expected) {
            throw std::runtime_error("Expected symbol: " + std::string(symbolToString(expected)));
        }

        auto symbol = jackTokenizer_.symbol();
        writeInlineTag("symbol", symbolToString(symbol));
        jackTokenizer_.advance();
        return symbol;
    }

    std::string CompilationEngine::compileType() {
        if (jackTokenizer_.tokenType() == Token::IDENTIFIER) {
            return compileIdentifier();
        }

        Keyword currKeyword = isOneOfKeywords({Keyword::Int, Keyword::Char, Keyword::Boolean});
        if (currKeyword == Keyword::Invalid) {
            throw std::runtime_error("Expected built-in type.\n");
        }
        return compileKeyword(currKeyword);
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

    void CompilationEngine::symbolTableDefine(const std::string& name, const std::string& type, SymbolKind kind) {
        try {
            symbolTable_.define(name, type, kind);
        }
        catch (const std::exception& ex) {
            throw std::runtime_error(jackTokenizer_.currentCursorLocation() + " | " + ex.what());
        }
    }
}
