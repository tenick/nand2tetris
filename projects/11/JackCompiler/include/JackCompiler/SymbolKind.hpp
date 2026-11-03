#pragma once

#include "JackCompiler/MemorySegment.hpp"
#include <cstdint>
#include <endian.h>
#include <stdexcept>
#include <string_view>

namespace JackCompiler {
    enum class SymbolKind : uint8_t {
        Invalid = 0,
        Static,
        Field,
        Arg,
        Var
    };

    constexpr std::string_view SYMBOL_KIND_STRINGS[] = {
        "invalid",
        "static",
        "field",
        "arg",
        "var"
    };

    inline constexpr std::string_view symbolKindToString(SymbolKind symbolKind) {
        return std::string_view(SYMBOL_KIND_STRINGS[static_cast<int>(symbolKind)]);
    }

    inline constexpr MemorySegment symbolKindToMemorySegment(SymbolKind symbolKind) {
        switch (symbolKind) {
            case SymbolKind::Arg:     return MemorySegment::M_ARGUMENT;
            case SymbolKind::Field:   return MemorySegment::M_THIS;
            case SymbolKind::Var:     return MemorySegment::M_LOCAL;
            case SymbolKind::Static:  return MemorySegment::M_STATIC;
            default:                  throw std::runtime_error("Symbol kind '" + std::string(symbolKindToString(symbolKind)) + "' has no memory segment conversion.\n");
        }
    }
}
