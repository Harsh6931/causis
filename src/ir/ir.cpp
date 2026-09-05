#include "ir/ir.h"

#include <sstream>

namespace causis::ir {

const char* opcode_name(Opcode opcode) {
  switch (opcode) {
  case Opcode::World:
    return "WORLD";
  case Opcode::Robot:
    return "ROBOT";
  case Opcode::Target:
    return "TARGET";
  case Opcode::Obstacle:
    return "OBSTACLE";
  case Opcode::Label:
    return "LABEL";
  case Opcode::Jump:
    return "JUMP";
  case Opcode::JumpIfFalse:
    return "JUMP_IF_FALSE";
  case Opcode::PushInt:
    return "PUSH_INT";
  case Opcode::PushBool:
    return "PUSH_BOOL";
  case Opcode::LoadVar:
    return "LOAD_VAR";
  case Opcode::StoreVar:
    return "STORE_VAR";
  case Opcode::Pop:
    return "POP";
  case Opcode::Add:
    return "ADD";
  case Opcode::Sub:
    return "SUB";
  case Opcode::Mul:
    return "MUL";
  case Opcode::Div:
    return "DIV";
  case Opcode::Eq:
    return "EQ";
  case Opcode::Ne:
    return "NE";
  case Opcode::Lt:
    return "LT";
  case Opcode::Le:
    return "LE";
  case Opcode::Gt:
    return "GT";
  case Opcode::Ge:
    return "GE";
  case Opcode::Not:
    return "NOT";
  case Opcode::Neg:
    return "NEG";
  case Opcode::LoadAgent:
    return "LOAD_AGENT";
  case Opcode::BeginTick:
    return "BEGIN_TICK";
  case Opcode::EndTick:
    return "END_TICK";
  case Opcode::MoveUp:
    return "MOVE_UP";
  case Opcode::MoveDown:
    return "MOVE_DOWN";
  case Opcode::MoveLeft:
    return "MOVE_LEFT";
  case Opcode::MoveRight:
    return "MOVE_RIGHT";
  case Opcode::MoveForward:
    return "MOVE_FORWARD";
  case Opcode::MoveToward:
    return "MOVE_TOWARD";
  case Opcode::TurnLeft:
    return "TURN_LEFT";
  case Opcode::TurnRight:
    return "TURN_RIGHT";
  case Opcode::Stop:
    return "STOP";
  case Opcode::DistanceTo:
    return "DISTANCE_TO";
  case Opcode::ObstacleAhead:
    return "OBSTACLE_AHEAD";
  case Opcode::Collision:
    return "COLLISION";
  }

  return "UNKNOWN";
}

std::string format_instruction(const Instruction& instruction) {
  std::ostringstream out;
  out << opcode_name(instruction.opcode);

  switch (instruction.opcode) {
  case Opcode::World:
    out << ' ' << instruction.int_value << ' ' << instruction.int_value2;
    break;
  case Opcode::Robot:
  case Opcode::Target:
    out << ' ' << instruction.text << ' ' << instruction.int_value << ' ' << instruction.int_value2;
    break;
  case Opcode::Obstacle:
    out << ' ' << instruction.int_value << ' ' << instruction.int_value2;
    break;
  case Opcode::Label:
  case Opcode::Jump:
  case Opcode::JumpIfFalse:
  case Opcode::LoadVar:
  case Opcode::StoreVar:
  case Opcode::LoadAgent:
    out << ' ' << instruction.text;
    break;
  case Opcode::PushInt:
    out << ' ' << instruction.int_value;
    break;
  case Opcode::PushBool:
    out << ' ' << (instruction.bool_value ? "true" : "false");
    break;
  case Opcode::MoveToward:
  case Opcode::DistanceTo:
    out << ' ' << instruction.text;
    break;
  default:
    break;
  }

  return out.str();
}

} // namespace causis::ir
