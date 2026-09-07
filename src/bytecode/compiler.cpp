#include "bytecode/compiler.h"

#include <unordered_map>

namespace causis::bytecode {
namespace {

class BytecodeCompiler {
public:
  CompileResult compile(const ir::IrProgram& program) {
    build_label_map(program.instructions);

    for (const ir::Instruction& instruction : program.instructions) {
      if (instruction.opcode == ir::Opcode::Label) {
        continue;
      }

      if (!emit_instruction(instruction)) {
        return make_error();
      }
    }

    Instruction halt;
    halt.opcode = Opcode::Halt;
    output_.code.push_back(halt);

    CompileResult result;
    result.ok = true;
    result.program = std::move(output_);
    return result;
  }

private:
  Program output_;
  std::unordered_map<std::string, std::size_t> label_to_pc_;
  std::optional<CompileError> error_;

  CompileResult make_error() const {
    CompileResult result;
    if (error_.has_value()) {
      result.error = *error_;
    } else {
      CompileError err;
      err.message = "bytecode compilation failed";
      result.error = err;
    }
    return result;
  }

  void report_error(int line, int column, const std::string& message) {
    if (!error_.has_value()) {
      CompileError err;
      err.message = message;
      err.line = line;
      err.column = column;
      error_ = err;
    }
  }

  bool has_error() const {
    return error_.has_value();
  }

  void build_label_map(const std::vector<ir::Instruction>& instructions) {
    std::size_t pc = 0;
    for (const ir::Instruction& instruction : instructions) {
      if (instruction.opcode == ir::Opcode::Label) {
        label_to_pc_[instruction.text] = pc;
        continue;
      }
      ++pc;
    }
  }

  int intern_string(const std::string& text) {
    for (std::size_t i = 0; i < output_.strings.size(); ++i) {
      if (output_.strings[i] == text) {
        return static_cast<int>(i);
      }
    }

    output_.strings.push_back(text);
    return static_cast<int>(output_.strings.size() - 1);
  }

  int intern_int(int32_t value) {
    for (std::size_t i = 0; i < output_.int_constants.size(); ++i) {
      if (output_.int_constants[i] == value) {
        return static_cast<int>(i);
      }
    }

    output_.int_constants.push_back(value);
    return static_cast<int>(output_.int_constants.size() - 1);
  }

  std::optional<std::size_t> resolve_label(const std::string& label, int line, int column) {
    const auto found = label_to_pc_.find(label);
    if (found == label_to_pc_.end()) {
      report_error(line, column, "unknown label '" + label + "'");
      return std::nullopt;
    }
    return found->second;
  }

  Instruction make_instruction(Opcode opcode, int line, int column) {
    Instruction instruction;
    instruction.opcode = opcode;
    instruction.line = line;
    instruction.column = column;
    return instruction;
  }

  bool emit_instruction(const ir::Instruction& instruction) {
    if (has_error()) {
      return false;
    }

    switch (instruction.opcode) {
    case ir::Opcode::PushInt: {
      Instruction out = make_instruction(Opcode::PushConst, instruction.line, instruction.column);
      out.a = intern_int(instruction.int_value);
      output_.code.push_back(out);
      return true;
    }
    case ir::Opcode::PushBool: {
      Instruction out = make_instruction(Opcode::PushBool, instruction.line, instruction.column);
      out.a = instruction.bool_value ? 1 : 0;
      output_.code.push_back(out);
      return true;
    }
    case ir::Opcode::LoadVar: {
      Instruction out = make_instruction(Opcode::LoadVar, instruction.line, instruction.column);
      out.a = intern_string(instruction.text);
      output_.code.push_back(out);
      return true;
    }
    case ir::Opcode::StoreVar: {
      Instruction out = make_instruction(Opcode::StoreVar, instruction.line, instruction.column);
      out.a = intern_string(instruction.text);
      output_.code.push_back(out);
      return true;
    }
    case ir::Opcode::Pop:
      output_.code.push_back(make_instruction(Opcode::Pop, instruction.line, instruction.column));
      return true;
    case ir::Opcode::Add:
      output_.code.push_back(make_instruction(Opcode::Add, instruction.line, instruction.column));
      return true;
    case ir::Opcode::Sub:
      output_.code.push_back(make_instruction(Opcode::Sub, instruction.line, instruction.column));
      return true;
    case ir::Opcode::Mul:
      output_.code.push_back(make_instruction(Opcode::Mul, instruction.line, instruction.column));
      return true;
    case ir::Opcode::Div:
      output_.code.push_back(make_instruction(Opcode::Div, instruction.line, instruction.column));
      return true;
    case ir::Opcode::Eq:
      output_.code.push_back(make_instruction(Opcode::Eq, instruction.line, instruction.column));
      return true;
    case ir::Opcode::Ne:
      output_.code.push_back(make_instruction(Opcode::Ne, instruction.line, instruction.column));
      return true;
    case ir::Opcode::Lt:
      output_.code.push_back(make_instruction(Opcode::Lt, instruction.line, instruction.column));
      return true;
    case ir::Opcode::Le:
      output_.code.push_back(make_instruction(Opcode::Le, instruction.line, instruction.column));
      return true;
    case ir::Opcode::Gt:
      output_.code.push_back(make_instruction(Opcode::Gt, instruction.line, instruction.column));
      return true;
    case ir::Opcode::Ge:
      output_.code.push_back(make_instruction(Opcode::Ge, instruction.line, instruction.column));
      return true;
    case ir::Opcode::Not:
      output_.code.push_back(make_instruction(Opcode::Not, instruction.line, instruction.column));
      return true;
    case ir::Opcode::Neg:
      output_.code.push_back(make_instruction(Opcode::Neg, instruction.line, instruction.column));
      return true;
    case ir::Opcode::Jump: {
      const std::optional<std::size_t> target =
          resolve_label(instruction.text, instruction.line, instruction.column);
      if (!target.has_value()) {
        return false;
      }
      Instruction out = make_instruction(Opcode::Jump, instruction.line, instruction.column);
      out.a = static_cast<int32_t>(*target);
      output_.code.push_back(out);
      return true;
    }
    case ir::Opcode::JumpIfFalse: {
      const std::optional<std::size_t> target =
          resolve_label(instruction.text, instruction.line, instruction.column);
      if (!target.has_value()) {
        return false;
      }
      Instruction out = make_instruction(Opcode::JumpIfFalse, instruction.line, instruction.column);
      out.a = static_cast<int32_t>(*target);
      output_.code.push_back(out);
      return true;
    }
    case ir::Opcode::World: {
      Instruction out = make_instruction(Opcode::CreateWorld, instruction.line, instruction.column);
      out.a = instruction.int_value;
      out.b = instruction.int_value2;
      output_.code.push_back(out);
      return true;
    }
    case ir::Opcode::Robot: {
      Instruction out = make_instruction(Opcode::CreateRobot, instruction.line, instruction.column);
      out.a = intern_string(instruction.text);
      out.b = instruction.int_value;
      out.c = instruction.int_value2;
      output_.code.push_back(out);
      return true;
    }
    case ir::Opcode::Target: {
      Instruction out = make_instruction(Opcode::CreateTarget, instruction.line, instruction.column);
      out.a = intern_string(instruction.text);
      out.b = instruction.int_value;
      out.c = instruction.int_value2;
      output_.code.push_back(out);
      return true;
    }
    case ir::Opcode::Obstacle: {
      Instruction out = make_instruction(Opcode::CreateObstacle, instruction.line, instruction.column);
      out.a = instruction.int_value;
      out.b = instruction.int_value2;
      output_.code.push_back(out);
      return true;
    }
    case ir::Opcode::LoadAgent: {
      Instruction out = make_instruction(Opcode::LoadAgent, instruction.line, instruction.column);
      out.a = intern_string(instruction.text);
      output_.code.push_back(out);
      return true;
    }
    case ir::Opcode::BeginTick:
      output_.code.push_back(
          make_instruction(Opcode::BeginTick, instruction.line, instruction.column));
      return true;
    case ir::Opcode::EndTick:
      output_.code.push_back(make_instruction(Opcode::EndTick, instruction.line, instruction.column));
      return true;
    case ir::Opcode::Stop:
      output_.code.push_back(make_instruction(Opcode::Stop, instruction.line, instruction.column));
      return true;
    case ir::Opcode::MoveUp:
      output_.code.push_back(make_instruction(Opcode::MoveUp, instruction.line, instruction.column));
      return true;
    case ir::Opcode::MoveDown:
      output_.code.push_back(
          make_instruction(Opcode::MoveDown, instruction.line, instruction.column));
      return true;
    case ir::Opcode::MoveLeft:
      output_.code.push_back(
          make_instruction(Opcode::MoveLeft, instruction.line, instruction.column));
      return true;
    case ir::Opcode::MoveRight:
      output_.code.push_back(
          make_instruction(Opcode::MoveRight, instruction.line, instruction.column));
      return true;
    case ir::Opcode::MoveForward:
      output_.code.push_back(
          make_instruction(Opcode::MoveForward, instruction.line, instruction.column));
      return true;
    case ir::Opcode::MoveToward: {
      Instruction out = make_instruction(Opcode::MoveToward, instruction.line, instruction.column);
      out.a = intern_string(instruction.text);
      output_.code.push_back(out);
      return true;
    }
    case ir::Opcode::TurnLeft:
      output_.code.push_back(
          make_instruction(Opcode::TurnLeft, instruction.line, instruction.column));
      return true;
    case ir::Opcode::TurnRight:
      output_.code.push_back(
          make_instruction(Opcode::TurnRight, instruction.line, instruction.column));
      return true;
    case ir::Opcode::DistanceTo: {
      Instruction out = make_instruction(Opcode::DistanceTo, instruction.line, instruction.column);
      out.a = intern_string(instruction.text);
      output_.code.push_back(out);
      return true;
    }
    case ir::Opcode::ObstacleAhead:
      output_.code.push_back(
          make_instruction(Opcode::ObstacleAhead, instruction.line, instruction.column));
      return true;
    case ir::Opcode::Collision:
      output_.code.push_back(
          make_instruction(Opcode::Collision, instruction.line, instruction.column));
      return true;
    case ir::Opcode::Label:
      return true;
    }

    report_error(instruction.line, instruction.column,
                 "unsupported IR opcode for bytecode compilation");
    return false;
  }
};

} // namespace

CompileResult compile_ir(const ir::IrProgram& program) {
  BytecodeCompiler compiler;
  return compiler.compile(program);
}

} // namespace causis::bytecode
