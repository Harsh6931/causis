#pragma once

#include "ir/ir.h"

namespace causis::ir {

// Run the Stage 8 optimizer pipeline on lowered IR.
// Pass 1: constant folding on the stack-machine instruction stream.
// Pass 2: dead-code elimination for branches with constant conditions.
IrProgram optimize_program(const IrProgram& program);

} // namespace causis::ir
