#include "runtime/tick_log.h"

#include <fstream>
#include <sstream>

namespace causis::runtime {
namespace {

std::string json_escape(const std::string& text) {
  std::ostringstream out;
  for (const char ch : text) {
    switch (ch) {
    case '\\':
      out << "\\\\";
      break;
    case '"':
      out << "\\\"";
      break;
    case '\n':
      out << "\\n";
      break;
    case '\r':
      out << "\\r";
      break;
    case '\t':
      out << "\\t";
      break;
    default:
      out << ch;
      break;
    }
  }
  return out.str();
}

std::vector<ObstacleSnapshot> collect_obstacles(const World& world) {
  std::vector<ObstacleSnapshot> obstacles;
  for (int y = 0; y < world.height(); ++y) {
    for (int x = 0; x < world.width(); ++x) {
      const Cell cell = world.cell_at(x, y);
      if (cell.kind == CellKind::Obstacle) {
        obstacles.push_back(ObstacleSnapshot{x, y});
      }
    }
  }
  return obstacles;
}

} // namespace

std::string direction_to_string(Direction direction) {
  switch (direction) {
  case Direction::Up:
    return "up";
  case Direction::Right:
    return "right";
  case Direction::Down:
    return "down";
  case Direction::Left:
    return "left";
  }

  return "right";
}

TickFrame capture_frame(const Simulation& simulation, int tick) {
  const World& world = simulation.world();

  TickFrame frame;
  frame.tick = tick;

  for (const Robot& robot : world.robots()) {
    RobotSnapshot snapshot;
    snapshot.name = robot.name;
    snapshot.x = robot.x;
    snapshot.y = robot.y;
    snapshot.direction = direction_to_string(robot.direction);
    snapshot.collision = robot.collision_flag;
    frame.robots.push_back(snapshot);
  }

  for (const Target& target : world.targets()) {
    TargetSnapshot snapshot;
    snapshot.name = target.name;
    snapshot.x = target.x;
    snapshot.y = target.y;
    frame.targets.push_back(snapshot);
  }

  frame.obstacles = collect_obstacles(world);
  return frame;
}

std::string tick_log_to_json(const TickLog& log) {
  std::ostringstream out;
  out << "{\n";
  out << "  \"width\": " << log.width << ",\n";
  out << "  \"height\": " << log.height << ",\n";
  out << "  \"frames\": [\n";

  for (std::size_t frame_index = 0; frame_index < log.frames.size(); ++frame_index) {
    const TickFrame& frame = log.frames[frame_index];
    out << "    {\n";
    out << "      \"tick\": " << frame.tick << ",\n";

    out << "      \"robots\": [\n";
    for (std::size_t i = 0; i < frame.robots.size(); ++i) {
      const RobotSnapshot& robot = frame.robots[i];
      out << "        {\"name\": \"" << json_escape(robot.name) << "\", "
          << "\"x\": " << robot.x << ", \"y\": " << robot.y << ", "
          << "\"direction\": \"" << json_escape(robot.direction) << "\", "
          << "\"collision\": " << (robot.collision ? "true" : "false") << "}";
      if (i + 1 < frame.robots.size()) {
        out << ',';
      }
      out << '\n';
    }
    out << "      ],\n";

    out << "      \"targets\": [\n";
    for (std::size_t i = 0; i < frame.targets.size(); ++i) {
      const TargetSnapshot& target = frame.targets[i];
      out << "        {\"name\": \"" << json_escape(target.name) << "\", "
          << "\"x\": " << target.x << ", \"y\": " << target.y << "}";
      if (i + 1 < frame.targets.size()) {
        out << ',';
      }
      out << '\n';
    }
    out << "      ],\n";

    out << "      \"obstacles\": [\n";
    for (std::size_t i = 0; i < frame.obstacles.size(); ++i) {
      const ObstacleSnapshot& obstacle = frame.obstacles[i];
      out << "        {\"x\": " << obstacle.x << ", \"y\": " << obstacle.y << "}";
      if (i + 1 < frame.obstacles.size()) {
        out << ',';
      }
      out << '\n';
    }
    out << "      ]\n";

    out << "    }";
    if (frame_index + 1 < log.frames.size()) {
      out << ',';
    }
    out << '\n';
  }

  out << "  ]\n";
  out << "}\n";
  return out.str();
}

bool write_tick_log_file(const std::string& path, const TickLog& log) {
  std::ofstream file(path);
  if (!file) {
    return false;
  }

  file << tick_log_to_json(log);
  return static_cast<bool>(file);
}

} // namespace causis::runtime
