#pragma once

#include <cstdint>
#include <string_view>

namespace JackCompiler {
    enum class MemorySegment : uint8_t {
        M_LOCAL = 0,
        M_ARGUMENT,
        M_THIS,
        M_THAT,
        M_CONSTANT,
        M_STATIC,
        M_POINTER,
        M_TEMP
    };

    constexpr std::string_view MEMORY_SEGMENT_STRINGS[] = {
        "local",
        "argument",
        "this",
        "that",
        "constant",
        "static",
        "pointer",
        "temp"
    };

    inline constexpr std::string_view memorySegmentToString(MemorySegment memorySegment) {
        return std::string_view(MEMORY_SEGMENT_STRINGS[static_cast<int>(memorySegment)]);
    }
}
