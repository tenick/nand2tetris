#pragma once

#include "JackCompiler/ArithmeticCommand.hpp"
#include "JackCompiler/LogicCommand.hpp"
#include "JackCompiler/MemorySegment.hpp"
#include <string_view>
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

namespace JackCompiler {
    class VMWriter {
    public:
        VMWriter(const fs::path&);
        void writePush(MemorySegment, int);
        void writePop(MemorySegment, int);
        void writeArithmetic(ArithmeticCommand);
        void writeLogic(LogicCommand);
        void writeLabel(std::string_view);
        void writeGoto(std::string_view);
        void writeIf(std::string_view);
        void writeCall(std::string_view, int);
        void writeFunction(std::string_view, int);
        void writeReturn();
        void close();
    private:
        std::ofstream outFile_;
    };
}
