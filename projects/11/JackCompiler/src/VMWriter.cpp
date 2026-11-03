#include "JackCompiler/VMWriter.hpp"
#include "JackCompiler/ArithmeticCommand.hpp"
#include "JackCompiler/LogicCommand.hpp"
#include "JackCompiler/MemorySegment.hpp"
#include <filesystem>
#include <string_view>

namespace fs = std::filesystem;

namespace JackCompiler {
    VMWriter::VMWriter(const fs::path& jackFilename) {
        std::string outputFileName = fs::path(jackFilename).replace_extension(".vm");
        outFile_.open(outputFileName);

        if (!outFile_.is_open()) {
            throw std::runtime_error("Failed to open '" + outputFileName + "'.\n");
        }
    }

    void VMWriter::writePush(MemorySegment memorySegment, int index) {
        outFile_ << "push " << memorySegmentToString(memorySegment) << " " << index << "\n";
    }
    void VMWriter::writePop(MemorySegment memorySegment, int index) {
        outFile_ << "pop " << memorySegmentToString(memorySegment) << " " << index << "\n";
    }
    void VMWriter::writeArithmetic(ArithmeticCommand arithmeticCommand) {
        outFile_ << arithmeticCommandToString(arithmeticCommand) << "\n";
    }
    void VMWriter::writeLogic(LogicCommand logicCommand) {
        outFile_ << logicCommandToString(logicCommand) << "\n";
    }
    void VMWriter::writeLabel(std::string_view label) {
        outFile_ << "label " << label << "\n";
    }
    void VMWriter::writeGoto(std::string_view label) {
        outFile_ << "goto " << label << "\n";
    }
    void VMWriter::writeIf(std::string_view label) {
        outFile_ << "if-goto " << label << "\n";
    }
    void VMWriter::writeCall(std::string_view functionName, int nArgs) {
        outFile_ << "call " << functionName << " " << nArgs << "\n";
    }
    void VMWriter::writeFunction(std::string_view functionName, int nVars) {
        outFile_ << "function " << functionName << " " << nVars << "\n";
    }
    void VMWriter::writeReturn() {
        outFile_ << "return\n";
    }
    void VMWriter::close() {
        outFile_.close();
    }
}
