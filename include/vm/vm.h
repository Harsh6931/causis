#pragma once

#include "bytecode/bytecode.h"
#include "runtime/simulation.h"

#include <optional>
#include <string>

namespace causis::vm {

struct VmError {
  std::string message;
  int line{1};
  int column{1};
};

struct VmResult {
  bool ok{false};
  std::optional<runtime::Simulation> simulation;
  std::optional<VmError> error;
};

// Execute bytecode produced by Stage 9. The host chooses how many tick frames
// (BEGIN_TICK ... END_TICK) to run before leaving the tick loop.
VmResult run_bytecode(const bytecode::Program& program, int tick_count);

} // namespace causis::vm
