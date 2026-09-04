#include "runtime/executor.h"

#include "runtime/program_runner.h"

#include <cmath>
#include <sstream>

namespace causis::runtime {

Executor::Executor(Simulation& simulation, const std::string& robot_name)
    : simulation_(simulation), robot_name_(robot_name) {}

void Executor::raise_error(int line, int column, const std::string& message) {
    RuntimeError err;
    err.message = message;
    err.line = line;
    err.column = column;
    throw RuntimeException(std::move(err));
}

void Executor::execute_block(const ast::BlockStmt& block) {
    for (const std::unique_ptr<ast::Stmt>& stmt : block.statements) {
        if (stopped_) {
            break;
        }

        execute_statement(*stmt);
    }
}

bool Executor::stopped() const {
    return stopped_;
}

void Executor::execute_statement(const ast::Stmt& stmt) {
    if (const auto* expr_stmt = dynamic_cast<const ast::ExprStmt*>(&stmt)) {
        evaluate_expression(*expr_stmt->expression);
        return;
    }

    if (const auto* block = dynamic_cast<const ast::BlockStmt*>(&stmt)) {
        execute_block(*block);
        return;
    }

    if (const auto* if_stmt = dynamic_cast<const ast::IfStmt*>(&stmt)) {
        if (evaluate_condition(*if_stmt->condition)) {
            execute_statement(*if_stmt->then_branch);
        } else if (if_stmt->else_branch != nullptr) {
            execute_statement(*if_stmt->else_branch);
        }
    }
}

Executor::Value Executor::evaluate_expression(const ast::Expr& expr) {
    if (const auto* literal = dynamic_cast<const ast::IntLiteralExpr*>(&expr)) {
        Value value;
        value.kind = Value::Kind::Int;
        value.int_value = literal->value;
        return value;
    }

    if (const auto* literal = dynamic_cast<const ast::BoolLiteralExpr*>(&expr)) {
        Value value;
        value.kind = Value::Kind::Bool;
        value.bool_value = literal->value;
        return value;
    }

    if (const auto* variable = dynamic_cast<const ast::VariableExpr*>(&expr)) {
        const auto found = variables_.find(variable->name);
        if (found == variables_.end()) {
            raise_error(variable->line, variable->column,
                        "unknown variable '" + variable->name + "'");
        }

        return found->second;
    }

    if (const auto* binary = dynamic_cast<const ast::BinaryExpr*>(&expr)) {
        return evaluate_binary(*binary);
    }

    if (const auto* unary = dynamic_cast<const ast::UnaryExpr*>(&expr)) {
        return evaluate_unary(*unary);
    }

    if (const auto* call = dynamic_cast<const ast::CallExpr*>(&expr)) {
        return evaluate_call(*call);
    }

    if (const auto* assign = dynamic_cast<const ast::AssignExpr*>(&expr)) {
        return evaluate_assignment(*assign);
    }

    raise_error(expr.line, expr.column, "unsupported expression");
}

bool Executor::evaluate_condition(const ast::Expr& expr) {
    return evaluate_bool(expr);
}

int Executor::evaluate_int(const ast::Expr& expr) {
    const Value value = evaluate_expression(expr);
    if (value.kind != Value::Kind::Int) {
        raise_error(expr.line, expr.column, "expected integer expression");
    }

    return value.int_value;
}

bool Executor::evaluate_bool(const ast::Expr& expr) {
    const Value value = evaluate_expression(expr);
    if (value.kind != Value::Kind::Bool) {
        raise_error(expr.line, expr.column, "expected boolean expression");
    }

    return value.bool_value;
}

int Executor::apply_int_binary(lexer::TokenType op, int left, int right, int line, int column) {
    switch (op) {
    case lexer::TokenType::Plus:
        return left + right;
    case lexer::TokenType::Minus:
        return left - right;
    case lexer::TokenType::Star:
        return left * right;
    case lexer::TokenType::Slash:
        if (right == 0) {
            raise_error(line, column, "division by zero");
        }
        return left / right;
    default:
        raise_error(line, column, "unsupported arithmetic operator");
    }
}

Executor::Value Executor::evaluate_binary(const ast::BinaryExpr& binary) {
    const Value left = evaluate_expression(*binary.left);
    const Value right = evaluate_expression(*binary.right);

    switch (binary.op) {
    case lexer::TokenType::Plus:
    case lexer::TokenType::Minus:
    case lexer::TokenType::Star:
    case lexer::TokenType::Slash: {
        if (left.kind != Value::Kind::Int || right.kind != Value::Kind::Int) {
            raise_error(binary.line, binary.column, "arithmetic operands must be integers");
        }

        Value result;
        result.kind = Value::Kind::Int;
        result.int_value =
            apply_int_binary(binary.op, left.int_value, right.int_value, binary.line, binary.column);
        return result;
    }

    case lexer::TokenType::Less:
    case lexer::TokenType::LessEqual:
    case lexer::TokenType::Greater:
    case lexer::TokenType::GreaterEqual: {
        if (left.kind != Value::Kind::Int || right.kind != Value::Kind::Int) {
            raise_error(binary.line, binary.column, "comparison operands must be integers");
        }

        bool comparison = false;
        switch (binary.op) {
        case lexer::TokenType::Less:
            comparison = left.int_value < right.int_value;
            break;
        case lexer::TokenType::LessEqual:
            comparison = left.int_value <= right.int_value;
            break;
        case lexer::TokenType::Greater:
            comparison = left.int_value > right.int_value;
            break;
        case lexer::TokenType::GreaterEqual:
            comparison = left.int_value >= right.int_value;
            break;
        default:
            break;
        }

        Value result;
        result.kind = Value::Kind::Bool;
        result.bool_value = comparison;
        return result;
    }

    case lexer::TokenType::EqualEqual:
    case lexer::TokenType::BangEqual: {
        if (left.kind != right.kind) {
            raise_error(binary.line, binary.column, "equality operands must have the same type");
        }

        bool equal = false;
        if (left.kind == Value::Kind::Int) {
            equal = left.int_value == right.int_value;
        } else {
            equal = left.bool_value == right.bool_value;
        }

        Value result;
        result.kind = Value::Kind::Bool;
        result.bool_value = binary.op == lexer::TokenType::EqualEqual ? equal : !equal;
        return result;
    }

    default:
        raise_error(binary.line, binary.column, "unsupported binary operator");
    }
}

Executor::Value Executor::evaluate_unary(const ast::UnaryExpr& unary) {
    const Value operand = evaluate_expression(*unary.operand);

    if (unary.op == lexer::TokenType::Bang) {
        if (operand.kind != Value::Kind::Bool) {
            raise_error(unary.line, unary.column, "logical not requires a boolean operand");
        }

        Value result;
        result.kind = Value::Kind::Bool;
        result.bool_value = !operand.bool_value;
        return result;
    }

    if (unary.op == lexer::TokenType::Minus) {
        if (operand.kind != Value::Kind::Int) {
            raise_error(unary.line, unary.column, "unary minus requires an integer operand");
        }

        Value result;
        result.kind = Value::Kind::Int;
        result.int_value = -operand.int_value;
        return result;
    }

    raise_error(unary.line, unary.column, "unsupported unary operator");
}

Executor::Value Executor::evaluate_assignment(const ast::AssignExpr& assign) {
    const Value value = evaluate_expression(*assign.value);
    if (value.kind != Value::Kind::Int && value.kind != Value::Kind::Bool) {
        raise_error(assign.line, assign.column, "assignment value must be integer or boolean");
    }

    variables_[assign.name] = value;
    return value;
}

Executor::Value Executor::evaluate_call(const ast::CallExpr& call) {
    World& world = simulation_.world();

    if (call.callee == "move_up") {
        world.move_up(robot_name_);
        return {};
    }
    if (call.callee == "move_down") {
        world.move_down(robot_name_);
        return {};
    }
    if (call.callee == "move_left") {
        world.move_left(robot_name_);
        return {};
    }
    if (call.callee == "move_right") {
        world.move_right(robot_name_);
        return {};
    }
    if (call.callee == "move_forward") {
        world.move_forward(robot_name_);
        return {};
    }
    if (call.callee == "turn_left") {
        world.turn_left(robot_name_);
        return {};
    }
    if (call.callee == "turn_right") {
        world.turn_right(robot_name_);
        return {};
    }
    if (call.callee == "stop") {
        stopped_ = true;
        return {};
    }

    if (call.callee == "move_toward") {
        if (call.arguments.size() != 1U) {
            raise_error(call.line, call.column, "move_toward expects one target argument");
        }

        const auto* target = dynamic_cast<const ast::VariableExpr*>(call.arguments[0].get());
        if (target == nullptr) {
            raise_error(call.line, call.column, "move_toward requires a target name");
        }

        world.move_toward(robot_name_, target->name);
        return {};
    }

    if (call.callee == "distance_to") {
        if (call.arguments.size() != 1U) {
            raise_error(call.line, call.column, "distance_to expects one target argument");
        }

        const auto* target = dynamic_cast<const ast::VariableExpr*>(call.arguments[0].get());
        if (target == nullptr) {
            raise_error(call.line, call.column, "distance_to requires a target name");
        }

        Value result;
        result.kind = Value::Kind::Int;
        result.int_value = world.distance_to(robot_name_, target->name);
        return result;
    }

    if (call.callee == "obstacle_ahead") {
        Value result;
        result.kind = Value::Kind::Bool;
        result.bool_value = world.obstacle_ahead(robot_name_);
        return result;
    }

    if (call.callee == "collision") {
        Value result;
        result.kind = Value::Kind::Bool;
        result.bool_value = world.collision(robot_name_);
        return result;
    }

    raise_error(call.line, call.column, "unknown function '" + call.callee + "'");
}

} // namespace causis::runtime
