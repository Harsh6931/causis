#pragma once

#include "bytecode/bytecode.h"

#include <string>

namespace causis::bytecode {

std::string disassemble_program(const Program& program);

} // namespace causis::bytecode
