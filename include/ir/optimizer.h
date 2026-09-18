#pragma once

#include "ir/ir.h"

#include <string>
#include <vector>

namespace causis::ir {

struct OptimizationStats {
  std::size_t ir_instructions_before{0};
  std::size_t ir_instructions_after{0};

  std::size_t bytecode_instructions_before{0};
  std::size_t bytecode_instructions_after{0};

  std::size_t ir_instructions_removed{0};
  std::size_t bytecode_instructions_removed{0};

  std::vector<std::string> passes_applied;
};

void finalize_optimization_stats(OptimizationStats& stats);

struct OptimizationResult {
  IrProgram program;
  OptimizationStats stats;
};

// Run constant folding, branch pruning, stop-unreachable, and redundant-turn passes.
OptimizationResult optimize_program_with_stats(const IrProgram& program);

IrProgram optimize_program(const IrProgram& program);

std::string format_optimization_stats(const OptimizationStats& stats,
                                      const std::string& program_path);

} // namespace causis::ir
