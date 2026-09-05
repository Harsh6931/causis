#include "ir/lower.h"

#include "ir/builder.h"

#include <sstream>
#include <unordered_map>

namespace causis::ir {
namespace {

class Lowerer {
public:
  explicit Lowerer(IrBuilder& builder) : builder_(builder) {}

  LowerResult lower(const ast::Program& program) {
    const ast::WorldDecl* world_decl = nullptr;
    std::vector<std::string> robot_order;
    std::unordered_map<std::string, const ast::BehaviorDecl*> behaviors;

    for (const std::unique_ptr<ast::Decl>& decl : program.declarations) {
      if (const auto* world = dynamic_cast<const ast::WorldDecl*>(decl.get())) {
        world_decl = world;
        continue;
      }

      if (const auto* robot = dynamic_cast<const ast::RobotDecl*>(decl.get())) {
        robot_order.push_back(robot->name);
        continue;
      }

      if (const auto* behavior = dynamic_cast<const ast::BehaviorDecl*>(decl.get())) {
        behaviors[behavior->robot_name] = behavior;
      }
    }

    if (world_decl == nullptr) {
      return make_error(1, 1, "program must declare a world");
    }

    builder_.emit_world(world_decl->width, world_decl->height, world_decl->line,
                        world_decl->column);

    for (const std::unique_ptr<ast::Decl>& decl : program.declarations) {
      if (const auto* robot = dynamic_cast<const ast::RobotDecl*>(decl.get())) {
        builder_.emit_robot(robot->name, robot->x, robot->y, robot->line, robot->column);
        continue;
      }

      if (const auto* target = dynamic_cast<const ast::TargetDecl*>(decl.get())) {
        builder_.emit_target(target->name, target->x, target->y, target->line, target->column);
        continue;
      }

      if (const auto* obstacle = dynamic_cast<const ast::ObstacleDecl*>(decl.get())) {
        builder_.emit_obstacle(obstacle->x, obstacle->y, obstacle->line, obstacle->column);
      }
    }

    // One simulation tick (docs/semantics.md section 7):
    //   BEGIN_TICK  — clear every robot's collision flag
    //   for each robot in declaration order:
    //     LOAD_AGENT + lowered every-tick body (movement applies immediately)
    //   END_TICK    — increment the global tick counter once
    // JUMP tick_loop repeats this frame; the VM/CLI chooses a finite tick count.
    // v1 only has "every tick", so event blocks are flattened into this frame.
    const std::string tick_loop = "tick_loop";
    builder_.emit_label(tick_loop, world_decl->line, world_decl->column);
    builder_.emit_begin_tick(world_decl->line, world_decl->column);

    for (const std::string& robot_name : robot_order) {
      if (has_error()) {
        break;
      }

      const auto found = behaviors.find(robot_name);
      if (found == behaviors.end()) {
        continue;
      }

      const std::string robot_end = builder_.new_label("robot_end");
      builder_.emit_load_agent(robot_name, found->second->line, found->second->column);

      for (const std::unique_ptr<ast::EveryTickStmt>& event : found->second->event_blocks) {
        if (event->body != nullptr) {
          lower_block(*event->body, robot_end);
        }
      }

      builder_.emit_label(robot_end, found->second->line, found->second->column);
    }

    if (has_error()) {
      return make_error(error_->line, error_->column, error_->message);
    }

    builder_.emit_end_tick(world_decl->line, world_decl->column);
    builder_.emit_jump(tick_loop, world_decl->line, world_decl->column);

    LowerResult result;
    result.ok = true;
    result.program = builder_.build();
    return result;
  }

private:
  IrBuilder& builder_;
  std::optional<LowerError> error_;

  static LowerResult make_error(int line, int column, const std::string& message) {
    LowerResult result;
    LowerError err;
    err.message = message;
    err.line = line;
    err.column = column;
    result.error = err;
    return result;
  }

  bool has_error() const {
    return error_.has_value();
  }

  void report_error(int line, int column, const std::string& message) {
    if (!error_.has_value()) {
      LowerError err;
      err.message = message;
      err.line = line;
      err.column = column;
      error_ = err;
    }
  }

  void lower_block(const ast::BlockStmt& block, const std::string& robot_end) {
    for (const std::unique_ptr<ast::Stmt>& stmt : block.statements) {
      if (has_error()) {
        return;
      }

      lower_statement(*stmt, robot_end);
    }
  }

  void lower_statement(const ast::Stmt& stmt, const std::string& robot_end) {
    if (has_error()) {
      return;
    }

    if (const auto* expr_stmt = dynamic_cast<const ast::ExprStmt*>(&stmt)) {
      if (dynamic_cast<const ast::AssignExpr*>(expr_stmt->expression.get()) != nullptr) {
        lower_expression(*expr_stmt->expression, robot_end);
      } else {
        lower_expression(*expr_stmt->expression, robot_end);
        if (!has_error()) {
          builder_.emit_pop(expr_stmt->line, expr_stmt->column);
        }
      }
      return;
    }

    if (const auto* block = dynamic_cast<const ast::BlockStmt*>(&stmt)) {
      lower_block(*block, robot_end);
      return;
    }

    if (const auto* if_stmt = dynamic_cast<const ast::IfStmt*>(&stmt)) {
      lower_expression(*if_stmt->condition, robot_end);
      if (has_error()) {
        return;
      }

      const std::string else_label = builder_.new_label("else");
      const std::string end_label = builder_.new_label("endif");

      builder_.emit_jump_if_false(else_label, if_stmt->line, if_stmt->column);
      lower_statement(*if_stmt->then_branch, robot_end);

      if (if_stmt->else_branch != nullptr) {
        if (!has_error()) {
          builder_.emit_jump(end_label, if_stmt->line, if_stmt->column);
          builder_.emit_label(else_label, if_stmt->line, if_stmt->column);
          lower_statement(*if_stmt->else_branch, robot_end);
          builder_.emit_label(end_label, if_stmt->line, if_stmt->column);
        }
      } else if (!has_error()) {
        builder_.emit_label(else_label, if_stmt->line, if_stmt->column);
      }
      return;
    }

    report_error(stmt.line, stmt.column, "unsupported statement");
  }

  void lower_expression(const ast::Expr& expr, const std::string& robot_end) {
    if (has_error()) {
      return;
    }

    if (const auto* literal = dynamic_cast<const ast::IntLiteralExpr*>(&expr)) {
      builder_.emit_push_int(literal->value, literal->line, literal->column);
      return;
    }

    if (const auto* literal = dynamic_cast<const ast::BoolLiteralExpr*>(&expr)) {
      builder_.emit_push_bool(literal->value, literal->line, literal->column);
      return;
    }

    if (const auto* variable = dynamic_cast<const ast::VariableExpr*>(&expr)) {
      builder_.emit_load_var(variable->name, variable->line, variable->column);
      return;
    }

    if (const auto* binary = dynamic_cast<const ast::BinaryExpr*>(&expr)) {
      lower_expression(*binary->left, robot_end);
      lower_expression(*binary->right, robot_end);
      if (!has_error()) {
        lower_binary(*binary);
      }
      return;
    }

    if (const auto* unary = dynamic_cast<const ast::UnaryExpr*>(&expr)) {
      lower_expression(*unary->operand, robot_end);
      if (!has_error()) {
        lower_unary(*unary);
      }
      return;
    }

    if (const auto* call = dynamic_cast<const ast::CallExpr*>(&expr)) {
      lower_call(*call, robot_end);
      return;
    }

    if (const auto* assign = dynamic_cast<const ast::AssignExpr*>(&expr)) {
      lower_expression(*assign->value, robot_end);
      if (!has_error()) {
        builder_.emit_store_var(assign->name, assign->line, assign->column);
      }
      return;
    }

    report_error(expr.line, expr.column, "unsupported expression");
  }

  void lower_binary(const ast::BinaryExpr& binary) {
    Opcode opcode = Opcode::Add;

    switch (binary.op) {
    case lexer::TokenType::Plus:
      opcode = Opcode::Add;
      break;
    case lexer::TokenType::Minus:
      opcode = Opcode::Sub;
      break;
    case lexer::TokenType::Star:
      opcode = Opcode::Mul;
      break;
    case lexer::TokenType::Slash:
      opcode = Opcode::Div;
      break;
    case lexer::TokenType::EqualEqual:
      opcode = Opcode::Eq;
      break;
    case lexer::TokenType::BangEqual:
      opcode = Opcode::Ne;
      break;
    case lexer::TokenType::Less:
      opcode = Opcode::Lt;
      break;
    case lexer::TokenType::LessEqual:
      opcode = Opcode::Le;
      break;
    case lexer::TokenType::Greater:
      opcode = Opcode::Gt;
      break;
    case lexer::TokenType::GreaterEqual:
      opcode = Opcode::Ge;
      break;
    default:
      report_error(binary.line, binary.column, "unsupported binary operator");
      return;
    }

    builder_.emit_binary(opcode, binary.line, binary.column);
  }

  void lower_unary(const ast::UnaryExpr& unary) {
    if (unary.op == lexer::TokenType::Bang) {
      builder_.emit_unary(Opcode::Not, unary.line, unary.column);
      return;
    }

    if (unary.op == lexer::TokenType::Minus) {
      builder_.emit_unary(Opcode::Neg, unary.line, unary.column);
      return;
    }

    report_error(unary.line, unary.column, "unsupported unary operator");
  }

  void lower_target_call(const ast::CallExpr& call, const std::string& builtin_name) {
    if (call.arguments.size() != 1U) {
      report_error(call.line, call.column,
                   "function '" + builtin_name + "' expects 1 argument, got " +
                       std::to_string(call.arguments.size()));
      return;
    }

    const auto* target = dynamic_cast<const ast::VariableExpr*>(call.arguments[0].get());
    if (target == nullptr) {
      report_error(call.arguments[0]->line, call.arguments[0]->column,
                   "function '" + builtin_name + "' requires a target name");
      return;
    }

    if (builtin_name == "move_toward") {
      builder_.emit_move_toward(target->name, call.line, call.column);
    } else {
      builder_.emit_distance_to(target->name, call.line, call.column);
    }
  }

  void lower_call(const ast::CallExpr& call, const std::string& robot_end) {
    if (call.callee == "move_up") {
      builder_.emit_move_up(call.line, call.column);
      return;
    }
    if (call.callee == "move_down") {
      builder_.emit_move_down(call.line, call.column);
      return;
    }
    if (call.callee == "move_left") {
      builder_.emit_move_left(call.line, call.column);
      return;
    }
    if (call.callee == "move_right") {
      builder_.emit_move_right(call.line, call.column);
      return;
    }
    if (call.callee == "move_forward") {
      builder_.emit_move_forward(call.line, call.column);
      return;
    }
    if (call.callee == "turn_left") {
      builder_.emit_turn_left(call.line, call.column);
      return;
    }
    if (call.callee == "turn_right") {
      builder_.emit_turn_right(call.line, call.column);
      return;
    }
    if (call.callee == "stop") {
      // Runtime control flow: STOP ends this robot's tick body when executed.
      // JUMP skips any remaining IR for this robot in the current tick frame.
      builder_.emit_stop(call.line, call.column);
      builder_.emit_jump(robot_end, call.line, call.column);
      return;
    }
    if (call.callee == "obstacle_ahead") {
      builder_.emit_obstacle_ahead(call.line, call.column);
      return;
    }
    if (call.callee == "collision") {
      builder_.emit_collision(call.line, call.column);
      return;
    }
    if (call.callee == "move_toward") {
      lower_target_call(call, "move_toward");
      return;
    }
    if (call.callee == "distance_to") {
      lower_target_call(call, "distance_to");
      return;
    }

    report_error(call.line, call.column, "unknown function '" + call.callee + "'");
  }
};

} // namespace

LowerResult lower_program(const ast::Program& program) {
  IrBuilder builder;
  Lowerer lowerer(builder);
  return lowerer.lower(program);
}

std::string print_ir(const IrProgram& program) {
  std::ostringstream out;

  for (const Instruction& instruction : program.instructions) {
    out << format_instruction(instruction) << '\n';
  }

  return out.str();
}

} // namespace causis::ir
