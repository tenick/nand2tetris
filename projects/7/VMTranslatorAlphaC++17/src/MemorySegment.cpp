#include "VMTranslatorAlpha/MemorySegment.hpp"

#include <unordered_map>

namespace {
    static const std::unordered_map<std::string, VMTranslator::MemorySegment> stringToMemorySegmentMap {
        {"local", VMTranslator::MemorySegment::M_LOCAL},
        {"argument", VMTranslator::MemorySegment::M_ARGUMENT},
        {"this", VMTranslator::MemorySegment::M_THIS},
        {"that", VMTranslator::MemorySegment::M_THAT},
        {"constant", VMTranslator::MemorySegment::M_CONSTANT},
        {"static", VMTranslator::MemorySegment::M_STATIC},
        {"pointer", VMTranslator::MemorySegment::M_POINTER},
        {"temp", VMTranslator::MemorySegment::M_TEMP}
    };
}

namespace VMTranslator {
    bool isStringAMemorySegment(const std::string& memSegText) {
        return stringToMemorySegmentMap.find(memSegText) != stringToMemorySegmentMap.end();
    }

    VMTranslator::MemorySegment stringToMemorySegment(const std::string& memSegText) {
        return stringToMemorySegmentMap.at(memSegText);
    }
}

