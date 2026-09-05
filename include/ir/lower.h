#pragma once

#include "ast/ast.h"
#include "ir/ir.h"

#include <optional>
#include <string>

namespace causis::ir {

struct LowerError {
  std::string message;
  int line{1};
  int column{1};
};

struct LowerResult {
  bool ok{false};
  std::optional<IrProgram> program;
  std::optional<LowerError> error;
};

// Lower a semantically valid AST into stack-based IR.
// Tick semantics: one BEGIN_TICK ... END_TICK frame wraps every robot's
// every-tick body (robots run in declaration order). JUMP tick_loop repeats
// that frame; how many ticks to run is decided by the host (VM / CLI), not IR.
LowerResult lower_program(const ast::Program& program);

std::string print_ir(const IrProgram& program);

} // namespace causis::ir
