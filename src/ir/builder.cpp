#include "ir/builder.h"

namespace causis::ir {

IrProgram IrBuilder::build() {
  return std::move(program_);
}

void IrBuilder::emit(Instruction instruction) {
  program_.instructions.push_back(std::move(instruction));
}

void IrBuilder::emit_world(int width, int height, int line, int column) {
  Instruction instruction;
  instruction.opcode = Opcode::World;
  instruction.int_value = width;
  instruction.int_value2 = height;
  instruction.line = line;
  instruction.column = column;
  emit(std::move(instruction));
}

void IrBuilder::emit_robot(const std::string& name, int x, int y, int line, int column) {
  Instruction instruction;
  instruction.opcode = Opcode::Robot;
  instruction.text = name;
  instruction.int_value = x;
  instruction.int_value2 = y;
  instruction.line = line;
  instruction.column = column;
  emit(std::move(instruction));
}

void IrBuilder::emit_target(const std::string& name, int x, int y, int line, int column) {
  Instruction instruction;
  instruction.opcode = Opcode::Target;
  instruction.text = name;
  instruction.int_value = x;
  instruction.int_value2 = y;
  instruction.line = line;
  instruction.column = column;
  emit(std::move(instruction));
}

void IrBuilder::emit_obstacle(int x, int y, int line, int column) {
  Instruction instruction;
  instruction.opcode = Opcode::Obstacle;
  instruction.int_value = x;
  instruction.int_value2 = y;
  instruction.line = line;
  instruction.column = column;
  emit(std::move(instruction));
}

std::string IrBuilder::new_label(const std::string& prefix) {
  return prefix + '_' + std::to_string(label_counter_++);
}

void IrBuilder::emit_label(const std::string& label, int line, int column) {
  Instruction instruction;
  instruction.opcode = Opcode::Label;
  instruction.text = label;
  instruction.line = line;
  instruction.column = column;
  emit(std::move(instruction));
}

void IrBuilder::emit_jump(const std::string& label, int line, int column) {
  Instruction instruction;
  instruction.opcode = Opcode::Jump;
  instruction.text = label;
  instruction.line = line;
  instruction.column = column;
  emit(std::move(instruction));
}

void IrBuilder::emit_jump_if_false(const std::string& label, int line, int column) {
  Instruction instruction;
  instruction.opcode = Opcode::JumpIfFalse;
  instruction.text = label;
  instruction.line = line;
  instruction.column = column;
  emit(std::move(instruction));
}

void IrBuilder::emit_push_int(int value, int line, int column) {
  Instruction instruction;
  instruction.opcode = Opcode::PushInt;
  instruction.int_value = value;
  instruction.line = line;
  instruction.column = column;
  emit(std::move(instruction));
}

void IrBuilder::emit_push_bool(bool value, int line, int column) {
  Instruction instruction;
  instruction.opcode = Opcode::PushBool;
  instruction.bool_value = value;
  instruction.line = line;
  instruction.column = column;
  emit(std::move(instruction));
}

void IrBuilder::emit_load_var(const std::string& name, int line, int column) {
  Instruction instruction;
  instruction.opcode = Opcode::LoadVar;
  instruction.text = name;
  instruction.line = line;
  instruction.column = column;
  emit(std::move(instruction));
}

void IrBuilder::emit_store_var(const std::string& name, int line, int column) {
  Instruction instruction;
  instruction.opcode = Opcode::StoreVar;
  instruction.text = name;
  instruction.line = line;
  instruction.column = column;
  emit(std::move(instruction));
}

void IrBuilder::emit_pop(int line, int column) {
  Instruction instruction;
  instruction.opcode = Opcode::Pop;
  instruction.line = line;
  instruction.column = column;
  emit(std::move(instruction));
}

void IrBuilder::emit_binary(Opcode opcode, int line, int column) {
  Instruction instruction;
  instruction.opcode = opcode;
  instruction.line = line;
  instruction.column = column;
  emit(std::move(instruction));
}

void IrBuilder::emit_unary(Opcode opcode, int line, int column) {
  Instruction instruction;
  instruction.opcode = opcode;
  instruction.line = line;
  instruction.column = column;
  emit(std::move(instruction));
}

void IrBuilder::emit_load_agent(const std::string& robot_name, int line, int column) {
  Instruction instruction;
  instruction.opcode = Opcode::LoadAgent;
  instruction.text = robot_name;
  instruction.line = line;
  instruction.column = column;
  emit(std::move(instruction));
}

void IrBuilder::emit_begin_tick(int line, int column) {
  Instruction instruction;
  instruction.opcode = Opcode::BeginTick;
  instruction.line = line;
  instruction.column = column;
  emit(std::move(instruction));
}

void IrBuilder::emit_end_tick(int line, int column) {
  Instruction instruction;
  instruction.opcode = Opcode::EndTick;
  instruction.line = line;
  instruction.column = column;
  emit(std::move(instruction));
}

void IrBuilder::emit_move_up(int line, int column) {
  emit({Opcode::MoveUp, {}, {}, 0, 0, false, line, column});
}

void IrBuilder::emit_move_down(int line, int column) {
  emit({Opcode::MoveDown, {}, {}, 0, 0, false, line, column});
}

void IrBuilder::emit_move_left(int line, int column) {
  emit({Opcode::MoveLeft, {}, {}, 0, 0, false, line, column});
}

void IrBuilder::emit_move_right(int line, int column) {
  emit({Opcode::MoveRight, {}, {}, 0, 0, false, line, column});
}

void IrBuilder::emit_move_forward(int line, int column) {
  emit({Opcode::MoveForward, {}, {}, 0, 0, false, line, column});
}

void IrBuilder::emit_move_toward(const std::string& target, int line, int column) {
  Instruction instruction;
  instruction.opcode = Opcode::MoveToward;
  instruction.text = target;
  instruction.line = line;
  instruction.column = column;
  emit(std::move(instruction));
}

void IrBuilder::emit_turn_left(int line, int column) {
  emit({Opcode::TurnLeft, {}, {}, 0, 0, false, line, column});
}

void IrBuilder::emit_turn_right(int line, int column) {
  emit({Opcode::TurnRight, {}, {}, 0, 0, false, line, column});
}

void IrBuilder::emit_stop(int line, int column) {
  emit({Opcode::Stop, {}, {}, 0, 0, false, line, column});
}

void IrBuilder::emit_distance_to(const std::string& target, int line, int column) {
  Instruction instruction;
  instruction.opcode = Opcode::DistanceTo;
  instruction.text = target;
  instruction.line = line;
  instruction.column = column;
  emit(std::move(instruction));
}

void IrBuilder::emit_obstacle_ahead(int line, int column) {
  emit({Opcode::ObstacleAhead, {}, {}, 0, 0, false, line, column});
}

void IrBuilder::emit_collision(int line, int column) {
  emit({Opcode::Collision, {}, {}, 0, 0, false, line, column});
}

} // namespace causis::ir
