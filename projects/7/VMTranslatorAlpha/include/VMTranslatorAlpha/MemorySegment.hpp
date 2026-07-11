#pragma once

#include <cstdint>
#include <string>

namespace VMTranslator {
    enum class MemorySegment : uint8_t {
        M_LOCAL,
        M_ARGUMENT,
        M_THIS,
        M_THAT,
        M_CONSTANT,
        M_STATIC,
        M_POINTER,
        M_TEMP
    };

    bool isStringAMemorySegment(const std::string&);
    MemorySegment stringToMemorySegment(const std::string&);
}
