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
        CodeWriter(const std::string& filePath);
        void writeCommand(CommandType, const std::string&, int);
        void writeComment(const std::string&);
        void close();
    private:
        void writeArithmetic(ArithmeticCommand);
        void writeLogic(LogicCommand);
        void writePush(const std::string&, int);
        void writePop(const std::string&, int);
        void writePushPop(CommandType, MemorySegment, int);
        void writeComp(LogicCommand);
        void writeAsmLines(const std::vector<std::string_view>&);
        std::ofstream outFile_;
        std::string inFileName_{};
        std::string outFilePath_{};
        int compCounter_{};
    };
}
