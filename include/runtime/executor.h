#pragma once

#include "ast/ast.h"
#include "runtime/simulation.h"

#include <string>
#include <unordered_map>

namespace causis::runtime {

// Tree-walking AST interpreter for one robot's behavior body. Used by program_runner
// until the bytecode VM (Stage 10) takes over execution.
class Executor {
public:
    Executor(Simulation& simulation, const std::string& robot_name);

    void execute_block(const ast::BlockStmt& block);
    bool stopped() const;

private:
    struct Value {
        enum class Kind { Int, Bool };
        Kind kind{Kind::Int};
        int int_value{0};
        bool bool_value{false};
    };

    Simulation& simulation_;
    std::string robot_name_;
    bool stopped_{false};
    std::unordered_map<std::string, Value> variables_;

    void execute_statement(const ast::Stmt& stmt);
    Value evaluate_expression(const ast::Expr& expr);
    Value evaluate_call(const ast::CallExpr& call);

    bool evaluate_condition(const ast::Expr& expr);
    int evaluate_int(const ast::Expr& expr);
    bool evaluate_bool(const ast::Expr& expr);

    Value evaluate_binary(const ast::BinaryExpr& binary);
    Value evaluate_unary(const ast::UnaryExpr& unary);
    Value evaluate_assignment(const ast::AssignExpr& assign);

    [[noreturn]] static void raise_error(int line, int column, const std::string& message);
    static int apply_int_binary(lexer::TokenType op, int left, int right, int line, int column);
};

} // namespace causis::runtime
