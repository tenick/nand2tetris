#pragma once

#include "JackCompiler/SymbolKind.hpp"
#include "JackCompiler/SymbolTableValue.hpp"
#include <string>
#include <string_view>
#include <unordered_map>

namespace JackCompiler {
    class SymbolTable {
    public:
        SymbolTable();
        void startSubroutine();
        void define(const std::string&, const std::string&, SymbolKind);
        int varCount(SymbolKind);
        SymbolKind kindOf(const std::string&);
        std::string typeOf(const std::string&);
        int indexOf(const std::string&);
        bool exists(std::string_view);
    private:
        std::unordered_map<std::string, SymbolTableValue> classScopeTable_;
        std::unordered_map<SymbolKind, int> classScopeKindCount_;
        std::unordered_map<std::string, SymbolTableValue> subroutineScopeTable_;
        std::unordered_map<SymbolKind, int> subroutineScopeKindCount_;
    };
}
