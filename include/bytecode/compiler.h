#pragma once

#include "bytecode/bytecode.h"
#include "ir/ir.h"

#include <optional>
#include <string>

namespace causis::bytecode {

struct CompileError {
  std::string message;
  int line{1};
  int column{1};
};

struct CompileResult {
  bool ok{false};
  std::optional<Program> program;
  std::optional<CompileError> error;
};

// Compile optimized IR into bytecode with resolved jump targets and constant pools.
CompileResult compile_ir(const ir::IrProgram& program);

} // namespace causis::bytecode
