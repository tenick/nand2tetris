#pragma once

#include "CommandType.hpp"

#include <string>

namespace VMTranslator {
    bool isStringABranchingCommand(const std::string& cmdText);
    CommandType stringToBranchingCommand(const std::string& cmdText);
}
