#pragma once

#include "JackCompiler/SymbolKind.hpp"
#include <string>

namespace JackCompiler {
    struct SymbolTableValue {
        SymbolTableValue() = default;
        SymbolTableValue(SymbolKind kind, std::string type, int index)
            : kind(kind), type(type), index(index) {}
        SymbolKind kind{};
        std::string type{};
        int index{};
    };
}
