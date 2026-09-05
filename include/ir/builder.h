#pragma once

#include "ir/ir.h"

#include <string>

namespace causis::ir {

class IrBuilder {
public:
  IrProgram build();

  void emit_world(int width, int height, int line, int column);
  void emit_robot(const std::string& name, int x, int y, int line, int column);
  void emit_target(const std::string& name, int x, int y, int line, int column);
  void emit_obstacle(int x, int y, int line, int column);

  std::string new_label(const std::string& prefix);
  void emit_label(const std::string& label, int line, int column);
  void emit_jump(const std::string& label, int line, int column);
  void emit_jump_if_false(const std::string& label, int line, int column);

  void emit_push_int(int value, int line, int column);
  void emit_push_bool(bool value, int line, int column);
  void emit_load_var(const std::string& name, int line, int column);
  void emit_store_var(const std::string& name, int line, int column);
  void emit_pop(int line, int column);

  void emit_binary(Opcode opcode, int line, int column);
  void emit_unary(Opcode opcode, int line, int column);

  void emit_load_agent(const std::string& robot_name, int line, int column);
  void emit_begin_tick(int line, int column);
  void emit_end_tick(int line, int column);

  void emit_move_up(int line, int column);
  void emit_move_down(int line, int column);
  void emit_move_left(int line, int column);
  void emit_move_right(int line, int column);
  void emit_move_forward(int line, int column);
  void emit_move_toward(const std::string& target, int line, int column);
  void emit_turn_left(int line, int column);
  void emit_turn_right(int line, int column);
  void emit_stop(int line, int column);

  void emit_distance_to(const std::string& target, int line, int column);
  void emit_obstacle_ahead(int line, int column);
  void emit_collision(int line, int column);

private:
  IrProgram program_;
  int label_counter_{0};

  void emit(Instruction instruction);
};

} // namespace causis::ir
