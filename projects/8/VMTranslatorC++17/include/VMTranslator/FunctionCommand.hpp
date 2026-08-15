#pragma once

#include "CommandType.hpp"

#include <string>

namespace VMTranslator {
    bool isStringAFunctionCommand(const std::string& cmdText);
    CommandType stringToFunctionCommand(const std::string& cmdText);
}

