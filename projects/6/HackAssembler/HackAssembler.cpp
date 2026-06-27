#include "HackAssembler.h"
#include <stdexcept>
#include <filesystem>
#include <bitset>
#include <algorithm>

namespace fs = std::filesystem;

HackAssembler::HackAssembler(const std::string& fileName) {
    asmFile_.open(fileName);
    if (!asmFile_.is_open()) {
        throw std::invalid_argument("File '" + fileName + "' is not found.");
    }

    fs::path outputFileNamePath{fileName};
    outputFileNamePath.replace_extension(".hack");
    std::string outputFileName = outputFileNamePath.string();
    binFile_.open(outputFileName);
    if (!binFile_.is_open()) {
        throw std::invalid_argument("Could not open or create the file '" + outputFileName + "'.");
    }
}
void HackAssembler::assemble() {
    // parsing of asm file, line by line
    std::string fileLine;
    while (std::getline(asmFile_, fileLine)) {
        std::istringstream iss(fileLine);
        std::string word;
        while (iss >> word) {
            if (word == "//") break;
            // reaching here means the line is either an actual statement or a label
            std::string binLine{parseLine(word)};
            if (!binLine.empty()) {
                binLines_.push_back(binLine);
            }
            break; // we assume each instruction have no whitespace inbetween
        }
    }

    // now assign memory address of found symbols that are not labels
    int memAddr = 16; // starts as 16, as per Hack spec
    for (auto& [lineNum, symbol] : unparsedSymbolLines_) {
        if (symbolTable_[symbol] == -1) {
            // means it's a variable
            symbolTable_[symbol] = memAddr;
            memAddr++;
        }
        binLines_[lineNum] = std::bitset<16>(symbolTable_[symbol]).to_string();
    }
    unparsedSymbolLines_.clear();

    // write to file
    for (auto& binLine : binLines_) {
        binFile_ << binLine << '\n';
    }
}
std::string HackAssembler::parseLine(const std::string& asmInstr) {
    if (asmInstr.empty()) return "";
    if (asmInstr[0] == '@') return parseAInstr(asmInstr);
    if (asmInstr[0] == '(') {
        handleLabel(asmInstr);
        return "";
    }
    return parseCInstr(asmInstr);
}
std::string HackAssembler::parseAInstr(const std::string& aInstr) {
    // a-instruction format: @value or @variable or @label
    std::string symbol{aInstr.substr(1, aInstr.size()-1)};
    int num{};
    if (isNumeric(symbol)) {
        num = std::stoi(symbol);
    }
    else {
        // means it's a variable
        if (symbolTable_.find(symbol) == symbolTable_.end()) {
            symbolTable_[symbol] = -1;
        }
        if (symbolTable_[symbol] == -1) {
            // delay the parsing
            unparsedSymbolLines_.push_back({binLines_.size(), symbol});
            return "toResolve"; // intermediate value, so it's added to lines (because empty lines are skipped)
        }
        num = symbolTable_[symbol];
    }

    return std::bitset<16>(num).to_string();
}
std::string HackAssembler::parseCInstr(const std::string& cInstr) {
    // c-instruction format: dest=comp;jump
    int compStartIdx = cInstr.find('=');
    int compEndIdx = cInstr.find(';');
    std::string dest{};
    std::string jump{};
    if (compStartIdx != std::string::npos) {
        dest = cInstr.substr(0, compStartIdx);
        compStartIdx++;
    }
    else compStartIdx = 0;
    if (compEndIdx != std::string::npos) {
        jump = cInstr.substr(compEndIdx+1, 3);
        compEndIdx--;
    }
    else compEndIdx = cInstr.size()-1;
    std::string comp{cInstr.substr(compStartIdx, compEndIdx-compStartIdx+1)};

    int binInstr = 0b111 << 13;
    binInstr |= (cInstrCompToBinMap_.at(comp) << 6);
    binInstr |= (cInstrDestToBinMap_.at(dest) << 3);
    binInstr |= cInstrJumpToBinMap_.at(jump);

    return std::bitset<16>(binInstr).to_string();
}
void HackAssembler::handleLabel(const std::string& label) {
    std::string symbol{label.substr(1, label.size()-2)};
    symbolTable_[symbol] = binLines_.size();
}
bool HackAssembler::isNumeric(const std::string& s) const {
    return !s.empty() && std::all_of(s.begin(), s.end(), ::isdigit);
}
