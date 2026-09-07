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

// Optional hook used by the visualizer to record world snapshots during execution.
struct VmTickRecorder {
  void* user_data{nullptr};
  void (*on_frame)(void* user_data, const runtime::Simulation& simulation, int tick){nullptr};
};

// Execute bytecode produced by Stage 9. The host chooses how many tick frames
// (BEGIN_TICK ... END_TICK) to run before leaving the tick loop.
VmResult run_bytecode(const bytecode::Program& program, int tick_count,
                      VmTickRecorder recorder = {});

} // namespace causis::vm
