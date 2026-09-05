#pragma once

#include <string>
#include <vector>

namespace causis::ir {

  // IR instruction vocabulary
enum class Opcode {
  World,
  Robot,
  Target,
  Obstacle,

  Label,
  Jump,
  JumpIfFalse,

  PushInt,
  PushBool,
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

  // Simulation context (see docs/semantics.md section 7).
  LoadAgent,  // select which robot runs the following behavior body
  BeginTick,  // start one global simulation tick (clear all collision flags)
  EndTick,    // finish that tick (increment global tick counter once)

  MoveUp,
  MoveDown,
  MoveLeft,
  MoveRight,
  MoveForward,
  MoveToward,
  TurnLeft,
  TurnRight,
  Stop,

  DistanceTo,
  ObstacleAhead,
  Collision,
};

// represent one IR instruction with opcode and operands along with location
struct Instruction {
  Opcode opcode{Opcode::Pop};
  std::string text;
  std::string text2;
  int int_value{0};
  int int_value2{0};
  bool bool_value{false};
  int line{1};
  int column{1};
};

// Flat instruction list produced by lowering. The tick_loop region uses one
// BEGIN_TICK / END_TICK pair per simulation tick, not per robot.
struct IrProgram {
  std::vector<Instruction> instructions;
};

const char* opcode_name(Opcode opcode);
std::string format_instruction(const Instruction& instruction);

} // namespace causis::ir
