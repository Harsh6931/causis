#include "bytecode/bytecode.h"

#include <sstream>

namespace causis::bytecode {

const char* opcode_name(Opcode opcode) {
  switch (opcode) {
  case Opcode::Halt:
    return "HALT";
  case Opcode::PushConst:
    return "PUSH_CONST";
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
  case Opcode::Jump:
    return "JUMP";
  case Opcode::JumpIfFalse:
    return "JUMP_IF_FALSE";
  case Opcode::CreateWorld:
    return "CREATE_WORLD";
  case Opcode::CreateRobot:
    return "CREATE_ROBOT";
  case Opcode::CreateTarget:
    return "CREATE_TARGET";
  case Opcode::CreateObstacle:
    return "CREATE_OBSTACLE";
  case Opcode::LoadAgent:
    return "LOAD_AGENT";
  case Opcode::BeginTick:
    return "BEGIN_TICK";
  case Opcode::EndTick:
    return "END_TICK";
  case Opcode::Stop:
    return "STOP";
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
  case Opcode::DistanceTo:
    return "DISTANCE_TO";
  case Opcode::ObstacleAhead:
    return "OBSTACLE_AHEAD";
  case Opcode::Collision:
    return "COLLISION";
  }

  return "UNKNOWN";
}

std::string format_instruction(const Program& program, const Instruction& instruction) {
  std::ostringstream out;
  out << opcode_name(instruction.opcode);

  switch (instruction.opcode) {
  case Opcode::PushConst:
    out << ' ' << instruction.a << " (" << program.int_constants[instruction.a] << ')';
    break;
  case Opcode::PushBool:
    out << ' ' << (instruction.a != 0 ? "true" : "false");
    break;
  case Opcode::LoadVar:
  case Opcode::StoreVar:
  case Opcode::LoadAgent:
    out << ' ' << instruction.a << " (\"" << program.strings[instruction.a] << "\")";
    break;
  case Opcode::MoveToward:
  case Opcode::DistanceTo:
    out << ' ' << instruction.a << " (\"" << program.strings[instruction.a] << "\")";
    break;
  case Opcode::Jump:
  case Opcode::JumpIfFalse:
    out << ' ' << instruction.a;
    break;
  case Opcode::CreateWorld:
    out << ' ' << instruction.a << ' ' << instruction.b;
    break;
  case Opcode::CreateRobot:
  case Opcode::CreateTarget:
    out << ' ' << instruction.a << " (\"" << program.strings[instruction.a] << "\") " << instruction.b
        << ' ' << instruction.c;
    break;
  case Opcode::CreateObstacle:
    out << ' ' << instruction.a << ' ' << instruction.b;
    break;
  default:
    break;
  }

  return out.str();
}

} // namespace causis::bytecode
