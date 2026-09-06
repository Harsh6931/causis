#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace causis::bytecode {

// Compact VM instruction set produced from optimized IR (Stage 9).
enum class Opcode : uint8_t {
  Halt,

  PushConst,
  LoadVar,
  StoreVar,
  Pop,

  Add,
  Sub,
  Mul,
  Div,
  Eq,
  Ne,
  Lt,
  Le,
  Gt,
  Ge,
  Not,
  Neg,

  Jump,
  JumpIfFalse,

  CreateWorld,
  CreateRobot,
  CreateTarget,
  CreateObstacle,

  LoadAgent,
  BeginTick,
  EndTick,
  Stop,

  MoveUp,
  MoveDown,
  MoveLeft,
  MoveRight,
  MoveForward,
  MoveToward,
  TurnLeft,
  TurnRight,

  DistanceTo,
  ObstacleAhead,
  Collision,
};

struct Instruction {
  Opcode opcode{Opcode::Halt};
  int32_t a{0};
  int32_t b{0};
  int32_t c{0};
  int line{1};
  int column{1};
};

// int_constants holds integer and boolean literals (0/1) for PushConst.
struct Program {
  std::vector<Instruction> code;
  std::vector<int32_t> int_constants;
  std::vector<std::string> strings;
};

const char* opcode_name(Opcode opcode);
std::string format_instruction(const Program& program, const Instruction& instruction);

} // namespace causis::bytecode
