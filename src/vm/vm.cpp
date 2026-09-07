#include "vm/vm.h"

#include <cmath>
#include <sstream>
#include <unordered_map>
#include <vector>

namespace causis::vm {
namespace {

struct VmException {
  VmError error;
};

struct Value {
  enum class Kind { Int, Bool } kind{Kind::Int};
  int int_value{0};
  bool bool_value{false};
};

class VirtualMachine {
public:
  explicit VirtualMachine(const bytecode::Program& program) : program_(program) {}

  VmResult run(int tick_count) {
    if (tick_count < 0) {
      return make_error(1, 1, "tick count must be non-negative");
    }

    max_ticks_ = tick_count;
    std::size_t ip = 0;

    try {
      while (ip < program_.code.size()) {
        const bytecode::Instruction& instruction = program_.code[ip];

        if (instruction.opcode == bytecode::Opcode::Halt) {
          if (!simulation_.has_value()) {
            return make_error(instruction.line, instruction.column,
                              "program halted before world was created");
          }

          VmResult result;
          result.ok = true;
          result.simulation = std::move(*simulation_);
          return result;
        }

        const std::size_t next_ip = execute(instruction, ip);
        ip = next_ip;
      }

      return make_error(1, 1, "program ended without HALT");
    } catch (const VmException& ex) {
      VmResult result;
      result.error = ex.error;
      return result;
    }
  }

private:
  const bytecode::Program& program_;
  std::optional<runtime::Simulation> simulation_;
  std::string current_robot_;
  std::unordered_map<std::string, Value> variables_;
  std::vector<Value> stack_;
  int ticks_completed_{0};
  int max_ticks_{0};

  [[noreturn]] void raise_error(int line, int column, const std::string& message) {
    throw VmException{VmError{message, line, column}};
  }

  VmResult make_error(int line, int column, const std::string& message) {
    VmResult result;
    result.error = VmError{message, line, column};
    return result;
  }

  runtime::World& world() {
    if (!simulation_.has_value()) {
      raise_error(1, 1, "world has not been created");
    }
    return simulation_->world();
  }

  void push_int(int value) {
    Value slot;
    slot.kind = Value::Kind::Int;
    slot.int_value = value;
    stack_.push_back(slot);
  }

  void push_bool(bool value) {
    Value slot;
    slot.kind = Value::Kind::Bool;
    slot.bool_value = value;
    stack_.push_back(slot);
  }

  Value pop_value(int line, int column) {
    if (stack_.empty()) {
      raise_error(line, column, "stack underflow");
    }

    const Value value = stack_.back();
    stack_.pop_back();
    return value;
  }

  int pop_int(int line, int column) {
    const Value value = pop_value(line, column);
    if (value.kind != Value::Kind::Int) {
      raise_error(line, column, "expected integer on stack");
    }
    return value.int_value;
  }

  bool pop_bool(int line, int column) {
    const Value value = pop_value(line, column);
    if (value.kind != Value::Kind::Bool) {
      raise_error(line, column, "expected boolean on stack");
    }
    return value.bool_value;
  }

  const std::string& string_at(int index, int line, int column) {
    if (index < 0 || static_cast<std::size_t>(index) >= program_.strings.size()) {
      raise_error(line, column, "invalid string pool index");
    }
    return program_.strings[static_cast<std::size_t>(index)];
  }

  int int_constant_at(int index, int line, int column) {
    if (index < 0 || static_cast<std::size_t>(index) >= program_.int_constants.size()) {
      raise_error(line, column, "invalid constant pool index");
    }
    return program_.int_constants[static_cast<std::size_t>(index)];
  }

  std::size_t jump_target(std::size_t ip, int32_t target) {
    if (target < 0 || static_cast<std::size_t>(target) >= program_.code.size()) {
      const bytecode::Instruction& instruction = program_.code[ip];
      raise_error(instruction.line, instruction.column, "jump target out of range");
    }
    return static_cast<std::size_t>(target);
  }

  std::size_t find_halt(std::size_t ip) {
    while (ip < program_.code.size() && program_.code[ip].opcode != bytecode::Opcode::Halt) {
      ++ip;
    }
    return ip;
  }

  std::size_t skip_tick_loop_after_max_ticks(std::size_t ip) {
    if (ip + 1 < program_.code.size() &&
        program_.code[ip + 1].opcode == bytecode::Opcode::Jump) {
      ip += 2;
    } else {
      ++ip;
    }

    return find_halt(ip);
  }

  std::size_t execute(const bytecode::Instruction& instruction, std::size_t ip) {
    switch (instruction.opcode) {
    case bytecode::Opcode::PushConst: {
      const int value = int_constant_at(instruction.a, instruction.line, instruction.column);
      push_int(value);
      return ip + 1;
    }
    case bytecode::Opcode::PushBool:
      push_bool(instruction.a != 0);
      return ip + 1;
    case bytecode::Opcode::LoadVar: {
      const std::string& name = string_at(instruction.a, instruction.line, instruction.column);
      const auto found = variables_.find(name);
      if (found == variables_.end()) {
        raise_error(instruction.line, instruction.column, "unknown variable '" + name + "'");
      }
      stack_.push_back(found->second);
      return ip + 1;
    }
    case bytecode::Opcode::StoreVar: {
      const std::string& name = string_at(instruction.a, instruction.line, instruction.column);
      const Value value = pop_value(instruction.line, instruction.column);
      variables_[name] = value;
      return ip + 1;
    }
    case bytecode::Opcode::Pop:
      if (!stack_.empty()) {
        pop_value(instruction.line, instruction.column);
      }
      return ip + 1;
    case bytecode::Opcode::Add: {
      const int right = pop_int(instruction.line, instruction.column);
      const int left = pop_int(instruction.line, instruction.column);
      push_int(left + right);
      return ip + 1;
    }
    case bytecode::Opcode::Sub: {
      const int right = pop_int(instruction.line, instruction.column);
      const int left = pop_int(instruction.line, instruction.column);
      push_int(left - right);
      return ip + 1;
    }
    case bytecode::Opcode::Mul: {
      const int right = pop_int(instruction.line, instruction.column);
      const int left = pop_int(instruction.line, instruction.column);
      push_int(left * right);
      return ip + 1;
    }
    case bytecode::Opcode::Div: {
      const int right = pop_int(instruction.line, instruction.column);
      const int left = pop_int(instruction.line, instruction.column);
      if (right == 0) {
        raise_error(instruction.line, instruction.column, "division by zero");
      }
      push_int(left / right);
      return ip + 1;
    }
    case bytecode::Opcode::Eq: {
      const Value right = pop_value(instruction.line, instruction.column);
      const Value left = pop_value(instruction.line, instruction.column);
      if (left.kind != right.kind) {
        raise_error(instruction.line, instruction.column,
                    "equality operands must have the same type");
      }
      const bool equal = left.kind == Value::Kind::Int ? left.int_value == right.int_value
                                                       : left.bool_value == right.bool_value;
      push_bool(equal);
      return ip + 1;
    }
    case bytecode::Opcode::Ne: {
      const Value right = pop_value(instruction.line, instruction.column);
      const Value left = pop_value(instruction.line, instruction.column);
      if (left.kind != right.kind) {
        raise_error(instruction.line, instruction.column,
                    "equality operands must have the same type");
      }
      const bool equal = left.kind == Value::Kind::Int ? left.int_value == right.int_value
                                                       : left.bool_value == right.bool_value;
      push_bool(!equal);
      return ip + 1;
    }
    case bytecode::Opcode::Lt: {
      const int right = pop_int(instruction.line, instruction.column);
      const int left = pop_int(instruction.line, instruction.column);
      push_bool(left < right);
      return ip + 1;
    }
    case bytecode::Opcode::Le: {
      const int right = pop_int(instruction.line, instruction.column);
      const int left = pop_int(instruction.line, instruction.column);
      push_bool(left <= right);
      return ip + 1;
    }
    case bytecode::Opcode::Gt: {
      const int right = pop_int(instruction.line, instruction.column);
      const int left = pop_int(instruction.line, instruction.column);
      push_bool(left > right);
      return ip + 1;
    }
    case bytecode::Opcode::Ge: {
      const int right = pop_int(instruction.line, instruction.column);
      const int left = pop_int(instruction.line, instruction.column);
      push_bool(left >= right);
      return ip + 1;
    }
    case bytecode::Opcode::Not: {
      const bool value = pop_bool(instruction.line, instruction.column);
      push_bool(!value);
      return ip + 1;
    }
    case bytecode::Opcode::Neg: {
      const int value = pop_int(instruction.line, instruction.column);
      push_int(-value);
      return ip + 1;
    }
    case bytecode::Opcode::Jump:
      return jump_target(ip, instruction.a);
    case bytecode::Opcode::JumpIfFalse: {
      const bool condition = pop_bool(instruction.line, instruction.column);
      if (!condition) {
        return jump_target(ip, instruction.a);
      }
      return ip + 1;
    }
    case bytecode::Opcode::CreateWorld:
      simulation_ = runtime::Simulation(runtime::World(instruction.a, instruction.b));
      return ip + 1;
    case bytecode::Opcode::CreateRobot: {
      const std::string& name = string_at(instruction.a, instruction.line, instruction.column);
      if (!world().place_robot(name, instruction.b, instruction.c)) {
        raise_error(instruction.line, instruction.column, "failed to place robot '" + name + "'");
      }
      return ip + 1;
    }
    case bytecode::Opcode::CreateTarget: {
      const std::string& name = string_at(instruction.a, instruction.line, instruction.column);
      if (!world().place_target(name, instruction.b, instruction.c)) {
        raise_error(instruction.line, instruction.column, "failed to place target '" + name + "'");
      }
      return ip + 1;
    }
    case bytecode::Opcode::CreateObstacle:
      if (!world().place_obstacle(instruction.a, instruction.b)) {
        raise_error(instruction.line, instruction.column, "failed to place obstacle");
      }
      return ip + 1;
    case bytecode::Opcode::LoadAgent: {
      current_robot_ = string_at(instruction.a, instruction.line, instruction.column);
      variables_.clear();
      return ip + 1;
    }
    case bytecode::Opcode::BeginTick:
      if (ticks_completed_ >= max_ticks_) {
        return find_halt(ip);
      }
      world();
      simulation_->begin_tick();
      return ip + 1;
    case bytecode::Opcode::EndTick:
      simulation_->end_tick();
      ++ticks_completed_;
      if (ticks_completed_ >= max_ticks_) {
        return skip_tick_loop_after_max_ticks(ip);
      }
      return ip + 1;
    case bytecode::Opcode::Stop:
      if (ip + 1 >= program_.code.size() ||
          program_.code[ip + 1].opcode != bytecode::Opcode::Jump) {
        raise_error(instruction.line, instruction.column,
                      "STOP is not followed by a jump to the robot end label");
      }
      return jump_target(ip, program_.code[ip + 1].a);
    case bytecode::Opcode::MoveUp:
      world().move_up(current_robot_);
      return ip + 1;
    case bytecode::Opcode::MoveDown:
      world().move_down(current_robot_);
      return ip + 1;
    case bytecode::Opcode::MoveLeft:
      world().move_left(current_robot_);
      return ip + 1;
    case bytecode::Opcode::MoveRight:
      world().move_right(current_robot_);
      return ip + 1;
    case bytecode::Opcode::MoveForward:
      world().move_forward(current_robot_);
      return ip + 1;
    case bytecode::Opcode::MoveToward: {
      const std::string& target = string_at(instruction.a, instruction.line, instruction.column);
      world().move_toward(current_robot_, target);
      return ip + 1;
    }
    case bytecode::Opcode::TurnLeft:
      world().turn_left(current_robot_);
      return ip + 1;
    case bytecode::Opcode::TurnRight:
      world().turn_right(current_robot_);
      return ip + 1;
    case bytecode::Opcode::DistanceTo: {
      const std::string& target = string_at(instruction.a, instruction.line, instruction.column);
      push_int(world().distance_to(current_robot_, target));
      return ip + 1;
    }
    case bytecode::Opcode::ObstacleAhead:
      push_bool(world().obstacle_ahead(current_robot_));
      return ip + 1;
    case bytecode::Opcode::Collision:
      push_bool(world().collision(current_robot_));
      return ip + 1;
    case bytecode::Opcode::Halt:
      return ip + 1;
    }

    raise_error(instruction.line, instruction.column, "unsupported bytecode opcode");
  }
};

} // namespace

VmResult run_bytecode(const bytecode::Program& program, int tick_count) {
  VirtualMachine vm(program);
  return vm.run(tick_count);
}

} // namespace causis::vm
