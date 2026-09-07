#pragma once

#include "runtime/simulation.h"

#include <string>
#include <vector>

namespace causis::runtime {

struct RobotSnapshot {
  std::string name;
  int x{0};
  int y{0};
  std::string direction;
  bool collision{false};
};

struct TargetSnapshot {
  std::string name;
  int x{0};
  int y{0};
};

struct ObstacleSnapshot {
  int x{0};
  int y{0};
};

struct TickFrame {
  int tick{0};
  std::vector<RobotSnapshot> robots;
  std::vector<TargetSnapshot> targets;
  std::vector<ObstacleSnapshot> obstacles;
};

struct TickLog {
  int width{0};
  int height{0};
  std::vector<TickFrame> frames;
};

std::string direction_to_string(Direction direction);

// Capture one simulation frame for the visualizer.
TickFrame capture_frame(const Simulation& simulation, int tick);

// Serialize a tick log to JSON for the static HTML visualizer.
std::string tick_log_to_json(const TickLog& log);

// Write JSON to a file. Returns false if the file could not be written.
bool write_tick_log_file(const std::string& path, const TickLog& log);

} // namespace causis::runtime
