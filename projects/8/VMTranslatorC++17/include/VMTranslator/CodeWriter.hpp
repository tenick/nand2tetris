#pragma once

#include "ArithmeticCommand.hpp"
#include "LogicCommand.hpp"
#include "CommandType.hpp"
#include "MemorySegment.hpp"

#include <fstream>
#include <vector>
#include <string>

namespace VMTranslator {
    class CodeWriter {
    public:
        CodeWriter(const std::string& outputFilePath, bool generateBootstrap=true);
        void setFileName(const std:: string&);
        void writeCommand(CommandType, const std::string&, int);
        void writeComment(const std::string&);
        void close();
    private:
        void writeInit();
        void writeArithmetic(ArithmeticCommand);
        void writeLogic(LogicCommand);
        void writeComp(LogicCommand);
        void writePush(const std::string&, int);
        void writePop(const std::string&, int);
        void writeBranch(CommandType, const std:: string&);
        void writeFunction(CommandType, const std:: string&, int);
        void writePushPop(CommandType, MemorySegment, int);
        void writeAsmLines(const std::vector<std::string_view>&);
        std::ofstream outFile_;
        std::string inFileName_{};
        std::string outFilePath_{};
        int compCounter_{};
        // function states
        std::string currFuncName_{};
        int currFuncRetCounter_{};
    };
}
