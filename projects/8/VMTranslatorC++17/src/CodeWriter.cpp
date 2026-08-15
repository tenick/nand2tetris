#include "VMTranslator/CodeWriter.hpp"
#include "VMTranslator/ArithmeticCommand.hpp"
#include "VMTranslator/CommandType.hpp"
#include "VMTranslator/LogicCommand.hpp"
#include "VMTranslator/MemorySegment.hpp"

#include <stdexcept>
#include <string>
#include <filesystem>
#include <vector>
#include <string_view>

namespace fs = std::filesystem; 

namespace {
    // segments: local, argument, this, that, and temp
    // all have similar assembly translations
    constexpr int kSegmentAsmIndex_ = 0;
    constexpr int kSegmentTempAsmIndex_ = 1;
    constexpr int kSegmentIAsmIndex_ = 2;
    const std::vector<std::string_view> PUSH_SEGMENT_ASM_ {
        "@segment", "D=M", "@i", "D=D+A", "A=D", "D=M",
        "@SP", "A=M", "M=D",
        "@SP", "M=M+1"
    };
    const std::vector<std::string_view> POP_SEGMENT_ASM_ {
        "@segment", "D=M", "@i", "D=D+A",
        "@POP_SEGMENT_I_VAR", "M=D",
        "@SP", "M=M-1", "A=M", "D=M",
        "@POP_SEGMENT_I_VAR", "A=M", "M=D"
    };

    constexpr int kPushConstantAsmIndex_ = 0;
    const std::vector<std::string_view> PUSH_CONSTANT_ASM_ {
        "@i", "D=A", "@SP", "A=M", "M=D",
        "@SP", "M=M+1"
    };

    constexpr int kPushStaticAsmIndex_ = 0;
    const std::vector<std::string_view> PUSH_STATIC_ASM_ {
        "@<fileName>.i", "D=M",
        "@SP", "A=M", "M=D",
        "@SP", "M=M+1"
    };

    constexpr int kPopStaticAsmIndex_ = 4;
    const std::vector<std::string_view> POP_STATIC_ASM_ {
        "@SP", "M=M-1", "A=M", "D=M",
        "@<fileName>.i", "M=D"
    };

    // push/pop pointer 0/1
    // 0 = THIS; 1 = THAT
    constexpr int kPushPointerAsmIndex_ = 0;
    const std::vector<std::string_view> PUSH_POINTER_ASM_ {
        "@T", "D=M",
        "@SP", "A=M", "M=D",
        "@SP", "M=M+1"
    };
    constexpr int kPopPointerAsmIndex_ = 4;
    const std::vector<std::string_view> POP_POINTER_ASM_ {
        "@SP", "M=M-1", "A=M", "D=M",
        "@T", "M=D"
    };

    // arithmetic and logical commands
    const std::vector<std::string_view> A_ADD_ASM_ {
        "@SP", "M=M-1", "A=M", "D=M",
        "@SP", "M=M-1", "A=M", "M=D+M",
        "@SP", "M=M+1"
    };
    const std::vector<std::string_view> A_NEG_ASM_ {
        "@SP", "M=M-1", "A=M", "M=-M",
        "@SP", "M=M+1"
    };

    enum class ComparisonAsmIndex : size_t {
        IS_NOT_COMP_JUMP        =8 ,
        JUMP_CMD                =9 ,
        COMP_BRANCH_END_JUMP    =12,
        IS_NOT_COMP_LABEL       =14,
        COMP_BRANCH_END_LABEL   =17,
    };
    const std::vector<std::string_view> L_COMP_TMPL_ASM_ {
        "@SP", "M=M-1", "A=M", "D=M",
        "@SP", "M=M-1", "A=M", "D=D-M",
        "@IS_NOT_ZERO", "D;JNE", "@0", "D=A-1",
        "@EQ_BRANCH_END", "0;JMP",
        "(IS_NOT_ZERO)", "@0", "D=A",
        "(EQ_BRANCH_END)", "@SP", "A=M", "M=D", "@SP", "M=M+1"
    };
    const std::vector<std::string_view> L_AND_ASM_ {
        "@SP", "M=M-1", "A=M", "D=M",
        "@SP", "M=M-1", "A=M", "M=D&M",
        "@SP", "M=M+1"
    };
    const std::vector<std::string_view> L_OR_ASM_ {
        "@SP", "M=M-1", "A=M", "D=M",
        "@SP", "M=M-1", "A=M", "M=D|M",
        "@SP", "M=M+1"
    };
    const std::vector<std::string_view> L_NOT_ASM_ {
        "@SP", "M=M-1", "A=M", "M=!M",
        "@SP", "M=M+1"
    };

    // branch commands
    // B_LABEL_ASM_ is simple, just declare the label
    constexpr int kGotoLabelAsmIndex_ = 0;
    const std::vector<std::string_view> B_GOTO_ASM_ {
        "@label", "0;JMP"
    };
    constexpr int kIfLabelAsmIndex_ = 4;
    const std::vector<std::string_view> B_IF_ASM_ {
        "@SP", "M=M-1", "A=M", "D=M",
        "@label", "D;JNE"
    };

    enum class FunctionCallAsmIndex : size_t {
        SAVE_RET_LBL =0 ,
        N_ARGS       =38,
        CALL_ADDR    =46,
        DECL_RET_LBL =48,
    };
    const std::vector<std::string_view> F_CALL_ASM_ {
        "@retLabel", "D=A", "@SP", "A=M", "M=D", "@SP", "M=M+1", // 7 (commands); 7 (total lines)
        "@LCL", "D=M", "@SP", "A=M", "M=D", "@SP", "M=M+1", // 7; 14
        "@ARG", "D=M", "@SP", "A=M", "M=D", "@SP", "M=M+1", // 7; 21
        "@THIS", "D=M", "@SP", "A=M", "M=D", "@SP", "M=M+1", // 7; 28
        "@THAT", "D=M", "@SP", "A=M", "M=D", "@SP", "M=M+1", // 7; 35
        "D=M", "@5", "D=D-A", "@nArgs", "D=D-A", "@ARG", "M=D", // 7; 42
        "@SP", "D=M", "@LCL", "M=D", // 4; 46
        "@callAddr", "0;JMP", // 2; 48
        "(retLabel)" // 1; 49
    };
    // F_FUNCTION_ASM_ is simple, just declare label and do `push constant 0` nVars times
    constexpr int kDoPopArg0 = 10;
    const std::vector<std::string_view> F_RETURN_ASM_ {
        "@LCL", "D=M", "@END_FRAME_VAR", "M=D", // endFrame = LCL
        "@5", "D=D-A", "A=D", "D=M", "@RET_ADDR_VAR", "M=D", // retAddr = *(endFrame-5)
        "pop argument 0", // re-use vm to asm for pop arg command
        "@ARG", "D=M+1", "@SP", "M=D",
        "@END_FRAME_VAR", "M=M-1", "A=M", "D=M", "@THAT", "M=D",
        "@END_FRAME_VAR", "M=M-1", "A=M", "D=M", "@THIS", "M=D",
        "@END_FRAME_VAR", "M=M-1", "A=M", "D=M", "@ARG", "M=D",
        "@END_FRAME_VAR", "M=M-1", "A=M", "D=M", "@LCL", "M=D",
        "@RET_ADDR_VAR", "A=M", "0;JMP"
    };
}

namespace VMTranslator {
    CodeWriter::CodeWriter(const std::string& outputfilePath, bool generateBootstrap) {
        fs::path fsFilePath{outputfilePath};
        outFilePath_ = fsFilePath.replace_extension(".asm").string();
        outFile_.open(outFilePath_);
        if (!outFile_.is_open()) {
            throw std::runtime_error("Failed to open '" + outputfilePath + "'.\n");
        }

        if (generateBootstrap)
            writeInit();
    }

    void CodeWriter::writeInit() {
        outFile_ << "// BOOTSTRAP START" << "\n";
        outFile_ << "// SP=256" << "\n";
        outFile_ << "// call Sys.init 0" << "\n";
        outFile_ << "@256\nD=A\n@SP\nM=D\n";
        writeFunction(CommandType::C_CALL, "Sys.init", 0);
        outFile_ << "// BOOTSTRAP END" << "\n";
    }

    void CodeWriter::setFileName(const std::string& vmFilePath) {
        inFileName_ = fs::path(vmFilePath).stem().string();
        outFile_ << "// " << inFileName_ << ".vm\n";
    }

    void CodeWriter::writeCommand(CommandType commandType, const std::string& arg1, int arg2) {
        switch (commandType) {
            case CommandType::C_PUSH:
                writePush(arg1, arg2);
                break;
            case CommandType::C_POP:
                writePop(arg1, arg2);
                break;
            case CommandType::C_ARITHMETIC:
                writeArithmetic(stringToArithmeticCommand(arg1));
                break;
            case CommandType::C_LOGIC:
                writeLogic(stringToLogicCommand(arg1));
                break;
            case CommandType::C_LABEL:
            case CommandType::C_GOTO:
            case CommandType::C_IF:
                writeBranch(commandType, arg1);
                break;
            case CommandType::C_CALL:
            case CommandType::C_FUNCTION:
            case CommandType::C_RETURN:
                writeFunction(commandType, arg1, arg2);
                break;
            default:
                throw std::runtime_error("Command not supported.\n");
        }
    }

    void CodeWriter::writeComment(const std::string& comment) {
        outFile_ << "// " << comment << "\n";
    }

    void CodeWriter::close() {
        outFile_.close();
    }

    void CodeWriter::writeArithmetic(ArithmeticCommand arithmeticCommand) {
        switch(arithmeticCommand) {
            case ArithmeticCommand::A_SUB:
                writeAsmLines(A_NEG_ASM_);
            case ArithmeticCommand::A_ADD:
                writeAsmLines(A_ADD_ASM_);
                break;
            case ArithmeticCommand::A_NEG:
                writeAsmLines(A_NEG_ASM_);
                break;
            default:
                throw std::runtime_error("Arithmetic Command not supported.\n");
        }
    }

    void CodeWriter::writeLogic(LogicCommand logicCommand) {
        switch(logicCommand) {
            case LogicCommand::L_AND:
                writeAsmLines(L_AND_ASM_);
                break;
            case LogicCommand::L_OR:
                writeAsmLines(L_OR_ASM_);
                break;
            case LogicCommand::L_NOT:
                writeAsmLines(L_NOT_ASM_);
                break;
            case LogicCommand::L_EQ:
            case LogicCommand::L_GT:
            case LogicCommand::L_LT:
                writeComp(logicCommand);
                break;
            default:
                throw std::runtime_error("Logic Command not supported.\n");
        }
    }

    void CodeWriter::writePush(const std::string& segmentStr, int index) {
        if (!isStringAMemorySegment(segmentStr)) {
            throw std::runtime_error("Unknown memory segment '" + segmentStr + "'.\n");
        }
        auto segment = stringToMemorySegment(segmentStr);
        writePushPop(CommandType::C_PUSH, segment, index);
    }

    void CodeWriter::writePop(const std::string& segmentStr, int index) {
        if (!isStringAMemorySegment(segmentStr)) {
            throw std::runtime_error("Unknown memory segment '" + segmentStr + "'.\n");
        }
        auto segment = stringToMemorySegment(segmentStr);
        if (segment == MemorySegment::M_CONSTANT) {
            throw std::runtime_error("Memory segment '" + segmentStr + "' does not support POP operation.\n");
        }
        writePushPop(CommandType::C_POP, segment, index);
    }

    void CodeWriter::writeBranch(CommandType cmdType, const std::string& label) {
        const std::string labelSymbol = currFuncName_ + "$" + label;
        switch (cmdType) {
            case CommandType::C_LABEL:
                outFile_ << "(" << labelSymbol << ")\n";
                break;
            case CommandType::C_GOTO:
                for (size_t i = 0; i < B_GOTO_ASM_.size(); i++) {
                    if (i == kGotoLabelAsmIndex_)
                        outFile_ << "@" << labelSymbol << "\n";
                    else
                        outFile_ <<  B_GOTO_ASM_[i] << "\n";
                }
                break;
            case CommandType::C_IF:
                for (size_t i = 0; i < B_IF_ASM_.size(); i++) {
                    if (i == kIfLabelAsmIndex_)
                        outFile_ << "@" << labelSymbol << "\n";
                    else
                        outFile_ <<  B_IF_ASM_[i] << "\n";
                }
                break;
            default:
                throw std::runtime_error("Unknown branch command\n");
        }
    }

    void CodeWriter::writePushPop(CommandType commandType, MemorySegment segment, int index) {
        std::string segmentAsm{};
        switch (segment) {
            case MemorySegment::M_ARGUMENT:
                segmentAsm = "ARG";
                break;
            case MemorySegment::M_LOCAL:
                segmentAsm = "LCL";
                break;
            case MemorySegment::M_THIS:
                segmentAsm = "THIS";
                break;
            case MemorySegment::M_THAT:
                segmentAsm = "THAT";
                break;
            case MemorySegment::M_TEMP:
                segmentAsm = "5";
                break;
            default: segmentAsm = "";
        }

        const std::vector<std::string_view>& segmentAsmLines = (commandType == CommandType::C_PUSH) 
            ? PUSH_SEGMENT_ASM_
            : POP_SEGMENT_ASM_;
        std::string tempSegmentAsm = "D=M";

        const std::vector<std::string_view>& staticAsmLines = (commandType == CommandType::C_PUSH) 
            ? PUSH_STATIC_ASM_
            : POP_STATIC_ASM_;
        const int staticAsmIndex = (commandType == CommandType::C_PUSH)
            ? kPushStaticAsmIndex_
            : kPopStaticAsmIndex_;

        const std::vector<std::string_view>& pointerAsmLines = (commandType == CommandType::C_PUSH) 
            ? PUSH_POINTER_ASM_ 
            : POP_POINTER_ASM_;

        const int pointerAsmIndex = (commandType == CommandType::C_PUSH)
            ? kPushPointerAsmIndex_ 
            : kPopPointerAsmIndex_;

        switch (segment) {
            case MemorySegment::M_TEMP:
                tempSegmentAsm = "D=A";
            case MemorySegment::M_ARGUMENT:
            case MemorySegment::M_LOCAL:
            case MemorySegment::M_THIS:
            case MemorySegment::M_THAT:
                for (size_t i = 0; i < segmentAsmLines.size(); i++) {
                    switch(i) {
                        case kSegmentAsmIndex_:
                            outFile_ << "@" << segmentAsm << "\n";
                            break;
                        case kSegmentTempAsmIndex_:
                            outFile_ << tempSegmentAsm << "\n";
                            break;
                        case kSegmentIAsmIndex_:
                            outFile_ << "@" << index << "\n";
                            break;
                        default:
                            outFile_ << segmentAsmLines[i] << '\n';
                            break;
                    }
                }
                break;
            case MemorySegment::M_CONSTANT:
                for (size_t i = 0; i < PUSH_CONSTANT_ASM_.size(); i++) {
                    if (i == kPushConstantAsmIndex_) {
                        outFile_ << "@" << index << "\n";
                    }
                    else outFile_ << PUSH_CONSTANT_ASM_[i] << '\n';
                }
                break;
            case MemorySegment::M_STATIC:
                for (size_t i = 0; i < staticAsmLines.size(); i++) {
                    if (i == staticAsmIndex) {
                        outFile_ << "@" << inFileName_ << "." << index << '\n';
                    }
                    else outFile_ << staticAsmLines[i] << '\n';
                }
                break;
            case MemorySegment::M_POINTER:
                for (size_t i = 0; i < pointerAsmLines.size(); i++) {
                    if (i == pointerAsmIndex) {
                        switch (index) {
                            case 0:
                                outFile_ << "@THIS\n";
                                break;
                            case 1:
                                outFile_ << "@THAT\n";
                                break;
                            default:
                                throw std::runtime_error("Invalid index '" + std::to_string(index) + "' for POINTER segment");
                        }
                    }
                    else outFile_ << pointerAsmLines[i] << '\n';
                }
                break;
            default:
                throw std::runtime_error("Memory segment not supported.\n");
        }
    }

    void CodeWriter::writeComp(LogicCommand logicCommand) {
        const char* labelStr = nullptr;
        const char* jumpCmd = nullptr;
        switch (logicCommand) {
            case LogicCommand::L_EQ:
                labelStr = "NOT_EQUAL";
                jumpCmd = "JNE";
                break;
            case LogicCommand::L_GT: 
                labelStr = "GREATER_THAN_OR_EQUAL";
                jumpCmd = "JGE";
                break;
            case LogicCommand::L_LT:
                labelStr = "LESS_THAN_OR_EQUAL";
                jumpCmd = "JLE";
                break;
            default:
                throw std::runtime_error("Logic Comparison Command not supported.\n");
        }
        for (size_t i = 0; i < L_COMP_TMPL_ASM_.size(); i++) {
            switch(static_cast<ComparisonAsmIndex>(i)) {
                case ComparisonAsmIndex::IS_NOT_COMP_JUMP:
                    outFile_ << "@IS_" << labelStr << "_" << compCounter_ << '\n';
                    break;
                case ComparisonAsmIndex::JUMP_CMD:
                    outFile_ << "D;" << jumpCmd << '\n';
                    break;
                case ComparisonAsmIndex::COMP_BRANCH_END_JUMP:
                    outFile_ << "@COMP_BRANCH_END_" << compCounter_ << '\n';
                    break;
                case ComparisonAsmIndex::IS_NOT_COMP_LABEL:
                    outFile_ << "(IS_" << labelStr << "_" << compCounter_ << ")\n";
                    break;
                case ComparisonAsmIndex::COMP_BRANCH_END_LABEL:
                    outFile_ << "(COMP_BRANCH_END_" << compCounter_ << ")\n";
                    break;
                default:
                    outFile_ << L_COMP_TMPL_ASM_[i] << '\n';
                    break;
            }
        }
        compCounter_++;
    }

    void CodeWriter::writeFunction(CommandType commandType, const std::string& functionName, int arg2) {
        switch (commandType) {
            case CommandType::C_CALL:
            {
                // arg2 means no. of args of called function
                std::string retLbl = currFuncName_ + "$" + std::to_string(currFuncRetCounter_++);
                for (size_t i = 0; i < F_CALL_ASM_.size(); i++) {
                    switch(static_cast<FunctionCallAsmIndex>(i)) {
                        case FunctionCallAsmIndex::SAVE_RET_LBL:
                            outFile_ << "@" << retLbl << "\n";
                            break;
                        case FunctionCallAsmIndex::CALL_ADDR:
                            outFile_ << "@" << functionName << "\n";
                            break;
                        case FunctionCallAsmIndex::N_ARGS:
                            outFile_ << "@" << arg2 << "\n";
                            break;
                        case FunctionCallAsmIndex::DECL_RET_LBL:
                            outFile_ << "(" << retLbl << ")\n";
                            break;
                        default:
                            outFile_ << F_CALL_ASM_[i] << '\n';
                            break;
                    }
                }
                break;
            }
            case CommandType::C_FUNCTION:
                // arg2 means no. of local variables of this function
                currFuncName_ = functionName;
                currFuncRetCounter_ = 0;
                outFile_ << "(" << functionName << ")\n";
                for (size_t i = 0; i < arg2; i++) {
                    writePushPop(CommandType::C_PUSH, MemorySegment::M_CONSTANT, 0);
                }
                break;
            case CommandType::C_RETURN:
                for (size_t i = 0; i < F_RETURN_ASM_.size(); i++) {
                    if (i == kDoPopArg0) {
                        writePushPop(CommandType::C_POP, MemorySegment::M_ARGUMENT, 0);
                    }
                    else {
                        outFile_ << F_RETURN_ASM_[i] << '\n';
                    }
                }
                break;
            default:
                throw std::runtime_error("Function Command not supported.\n");
        }
    }

    void CodeWriter::writeAsmLines(const std::vector<std::string_view>& asmLines) {
        for (size_t i = 0; i < asmLines.size(); i++) {
            outFile_ << asmLines[i] << '\n';
        }
    }
}
