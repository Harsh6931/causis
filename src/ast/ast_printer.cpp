#include "ast/ast.h"

#include <sstream>
#include <string>

namespace causis::ast {
namespace {

std::string print_expr(const Expr& expr);

std::string op_display_name(lexer::TokenType op) {
  switch (op) {
  case lexer::TokenType::Equal:
    return "=";
  case lexer::TokenType::EqualEqual:
    return "==";
  case lexer::TokenType::BangEqual:
    return "!=";
  case lexer::TokenType::Less:
    return "<";
  case lexer::TokenType::LessEqual:
    return "<=";
  case lexer::TokenType::Greater:
    return ">";
  case lexer::TokenType::GreaterEqual:
    return ">=";
  case lexer::TokenType::Bang:
    return "!";
  default:
    return lexer::token_type_name(op);
  }
}

std::string call_args(const CallExpr& call) {
  std::ostringstream out;
  out << call.callee << '(';
  for (std::size_t i = 0; i < call.arguments.size(); ++i) {
    if (i > 0) {
      out << ", ";
    }
    out << print_expr(*call.arguments[i]);
  }
  out << ')';
  return out.str();
}

std::string print_expr(const Expr& expr) {
  if (const auto* literal = dynamic_cast<const IntLiteralExpr*>(&expr)) {
    return std::to_string(literal->value);
  }
  if (const auto* literal = dynamic_cast<const BoolLiteralExpr*>(&expr)) {
    return literal->value ? "true" : "false";
  }
  if (const auto* variable = dynamic_cast<const VariableExpr*>(&expr)) {
    return variable->name;
  }
  if (const auto* call = dynamic_cast<const CallExpr*>(&expr)) {
    return call_args(*call);
  }
  if (const auto* binary = dynamic_cast<const BinaryExpr*>(&expr)) {
    return print_expr(*binary->left) + ' ' + op_display_name(binary->op) + ' ' +
           print_expr(*binary->right);
  }
  if (const auto* unary = dynamic_cast<const UnaryExpr*>(&expr)) {
    return op_display_name(unary->op) + print_expr(*unary->operand);
  }
  if (const auto* assign = dynamic_cast<const AssignExpr*>(&expr)) {
    return assign->name + " = " + print_expr(*assign->value);
  }
  return "?";
}

void append_line_ref(std::ostringstream& out, int line) {
  if (line > 0) {
    out << "  (line " << line << ')';
  }
}

void writeln(std::ostringstream& out, const std::string& indent, const std::string& text, int line) {
  out << indent << text;
  append_line_ref(out, line);
  out << '\n';
}

void print_block(std::ostringstream& out, const BlockStmt& block, const std::string& indent);

void print_stmt(std::ostringstream& out, const Stmt& stmt, const std::string& indent) {
  if (const auto* expr_stmt = dynamic_cast<const ExprStmt*>(&stmt)) {
    writeln(out, indent, print_expr(*expr_stmt->expression) + ';', stmt.line);
    return;
  }

  if (const auto* block = dynamic_cast<const BlockStmt*>(&stmt)) {
    print_block(out, *block, indent);
    return;
  }

  if (const auto* if_stmt = dynamic_cast<const IfStmt*>(&stmt)) {
    writeln(out, indent, "if " + print_expr(*if_stmt->condition), stmt.line);
    print_stmt(out, *if_stmt->then_branch, indent + "  ");
    if (if_stmt->else_branch) {
      writeln(out, indent, "else", 0);
      print_stmt(out, *if_stmt->else_branch, indent + "  ");
    }
    return;
  }

  if (const auto* every_tick = dynamic_cast<const EveryTickStmt*>(&stmt)) {
    writeln(out, indent, "every tick", stmt.line);
    print_block(out, *every_tick->body, indent + "  ");
  }
}

void print_block(std::ostringstream& out, const BlockStmt& block, const std::string& indent) {
  for (const std::unique_ptr<Stmt>& statement : block.statements) {
    print_stmt(out, *statement, indent);
  }
}


std::string decl_label(const Decl& decl) {
  if (const auto* world = dynamic_cast<const WorldDecl*>(&decl)) {
    return "world " + std::to_string(world->width) + ' ' + std::to_string(world->height) + ';';
  }
  if (const auto* robot = dynamic_cast<const RobotDecl*>(&decl)) {
    return "robot " + robot->name + " at " + std::to_string(robot->x) + ' ' +
           std::to_string(robot->y) + ';';
  }
  if (const auto* target = dynamic_cast<const TargetDecl*>(&decl)) {
    return "target " + target->name + " at " + std::to_string(target->x) + ' ' +
           std::to_string(target->y) + ';';
  }
  if (const auto* obstacle = dynamic_cast<const ObstacleDecl*>(&decl)) {
    return "obstacle at " + std::to_string(obstacle->x) + ' ' + std::to_string(obstacle->y) + ';';
  }
  return "?";
}

} // namespace

std::string print_program(const Program& program, int /*indent*/) {
  std::ostringstream out;

  out << "causis AST\n";
  out << "==========\n";
  out << "Program\n";

  const std::size_t count = program.declarations.size();
  for (std::size_t i = 0; i < count; ++i) {
    const Decl& decl = *program.declarations[i];
    const bool last = i + 1 == count;
    const std::string branch = last ? "\\-- " : "|-- ";
    const std::string child_indent = last ? "    " : "|   ";

    if (const auto* behavior = dynamic_cast<const BehaviorDecl*>(&decl)) {
      writeln(out, "  " + branch, "behavior " + behavior->robot_name, decl.line);
      for (const std::unique_ptr<EveryTickStmt>& event : behavior->event_blocks) {
        print_stmt(out, *event, "  " + child_indent + "  ");
      }
      continue;
    }

    writeln(out, "  " + branch, decl_label(decl), decl.line);
  }

  out << '\n';
  out << count << " top-level declaration(s)\n";

  return out.str();
}

} // namespace causis::ast
