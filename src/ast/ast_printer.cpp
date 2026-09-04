
//Take AST Created by parser and convert to human readable text
// Basically the tree structure
#include "ast/ast.h"

#include <sstream>
#include <string>

namespace causis::ast {
namespace {

std::string indent_prefix(int indent, bool is_last) { //create tree drawing structure
    std::string prefix;
    for (int i = 0; i < indent - 1; ++i) {
        prefix += " │  ";
    }
    if (indent > 0) {
        prefix += is_last ? " └── " : " ├── ";
    }
    return prefix;
}

std::string print_expr(const Expr& expr);

std::string call_args(const CallExpr& call) {  //print function call arguments eg moveForward(1)
    std::ostringstream out;
    out << call.callee << '(';
    for (std::size_t i = 0; i < call.arguments.size(); ++i) {
        if (i > 0) {
            out << ',';
        }
        out << print_expr(*call.arguments[i]);
    }
    out << ')';
    return out.str();
}

std::string print_expr(const Expr& expr) { //check expression and convert to string and print it
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
        return print_expr(*binary->left) + " " + lexer::token_type_name(binary->op) + " " +
               print_expr(*binary->right);
    }
    if (const auto* unary = dynamic_cast<const UnaryExpr*>(&expr)) {
        return lexer::token_type_name(unary->op) + print_expr(*unary->operand);
    }
    if (const auto* assign = dynamic_cast<const AssignExpr*>(&expr)) {
        return assign->name + " = " + print_expr(*assign->value);
    } 
    return "?";  //fallback for unknown expression
}

void print_stmt(std::ostringstream& out, const Stmt& stmt, int indent, bool is_last);

void print_block(std::ostringstream& out, const BlockStmt& block, int indent) {
    for (std::size_t i = 0; i < block.statements.size(); ++i) {
        const bool last = i + 1 == block.statements.size();
        print_stmt(out, *block.statements[i], indent, last);
    }
}

void print_stmt(std::ostringstream& out, const Stmt& stmt, int indent, bool is_last) {
    const std::string prefix = indent_prefix(indent, is_last);

    if (const auto* expr_stmt = dynamic_cast<const ExprStmt*>(&stmt)) {
        out << prefix << print_expr(*expr_stmt->expression) << '\n';
        return;
    }

    if (const auto* block = dynamic_cast<const BlockStmt*>(&stmt)) {
        out << prefix << "Block\n";
        print_block(out, *block, indent + 1);
        return;
    }

    if (const auto* if_stmt = dynamic_cast<const IfStmt*>(&stmt)) {
        out << prefix << "If " << print_expr(*if_stmt->condition) << '\n';
        print_stmt(out, *if_stmt->then_branch, indent + 1, if_stmt->else_branch == nullptr);
        if (if_stmt->else_branch) {
            out << indent_prefix(indent + 1, true) << "Else\n";
            print_stmt(out, *if_stmt->else_branch, indent + 2, true);
        }
        return;
    }

    if (const auto* every_tick = dynamic_cast<const EveryTickStmt*>(&stmt)) {
        out << prefix << "EveryTick\n";
        print_block(out, *every_tick->body, indent + 1);
    }
}

void print_decl(std::ostringstream& out, const Decl& decl, int indent, bool is_last) { // print for entities
    const std::string prefix = indent_prefix(indent, is_last);

    if (const auto* world = dynamic_cast<const WorldDecl*>(&decl)) {
        out << prefix << "World(" << world->width << ',' << world->height << ")\n";
        return;
    }

    if (const auto* robot = dynamic_cast<const RobotDecl*>(&decl)) {
        out << prefix << "Robot(" << robot->name << ',' << robot->x << ',' << robot->y << ")\n";
        return;
    }

    if (const auto* target = dynamic_cast<const TargetDecl*>(&decl)) {
        out << prefix << "Target(" << target->name << ',' << target->x << ',' << target->y << ")\n";
        return;
    }

    if (const auto* obstacle = dynamic_cast<const ObstacleDecl*>(&decl)) {
        out << prefix << "Obstacle(" << obstacle->x << ',' << obstacle->y << ")\n";
        return;
    }

    if (const auto* behavior = dynamic_cast<const BehaviorDecl*>(&decl)) {
        out << prefix << "Behavior(" << behavior->robot_name << ")\n";
        for (std::size_t i = 0; i < behavior->event_blocks.size(); ++i) {
            const bool last = i + 1 == behavior->event_blocks.size();
            print_stmt(out, *behavior->event_blocks[i], indent + 1, last);
        }
    }
}

} // namespace

std::string print_program(const Program& program, int indent) {
    std::ostringstream out;

    for (std::size_t i = 0; i < program.declarations.size(); ++i) {
        const bool last = i + 1 == program.declarations.size();
        print_decl(out, *program.declarations[i], indent, last);
    }

    return out.str();
}

} // namespace causis::ast
