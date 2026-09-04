#include "semantic/analyzer.h"

#include <sstream>
#include <unordered_map>

namespace causis::semantic {
namespace {

struct BuiltinInfo {
    int arg_count{0};
    ValueType return_type{ValueType::Void};
    bool needs_target_arg{false};
};

const std::unordered_map<std::string, BuiltinInfo>& builtins() {
    static const std::unordered_map<std::string, BuiltinInfo> table = {
        {"move_up", {0, ValueType::Void, false}},
        {"move_down", {0, ValueType::Void, false}},
        {"move_left", {0, ValueType::Void, false}},
        {"move_right", {0, ValueType::Void, false}},
        {"move_forward", {0, ValueType::Void, false}},
        {"move_toward", {1, ValueType::Void, true}},
        {"turn_left", {0, ValueType::Void, false}},
        {"turn_right", {0, ValueType::Void, false}},
        {"stop", {0, ValueType::Void, false}},
        {"distance_to", {1, ValueType::Int, true}},
        {"obstacle_ahead", {0, ValueType::Bool, false}},
        {"collision", {0, ValueType::Bool, false}},
    };
    return table;
}

} // namespace

SemanticResult Analyzer::analyze(const ast::Program& program) {
    has_world_ = false;
    world_width_ = 0;
    world_height_ = 0;
    robots_.clear();
    targets_.clear();
    variables_.clear();
    occupied_cells_.clear();
    robots_with_behavior_.clear();
    in_behavior_ = false;
    error_.reset();

    //analyze all declarations in the program
    for (const std::unique_ptr<ast::Decl>& decl : program.declarations) {
        analyze_declaration(*decl);
        if (has_error()) {
            break;
        }
    }

    // World is mandatory to declare first in Our program (langauge rule)
    SemanticResult result;
    if (!has_world_ && !has_error()) {
        report_error(1, 1, "program must declare exactly one world");
    }

    if (has_error()) {
        result.error = error_;
        return result;
    }

    result.ok = true;
    return result;
}

void Analyzer::report_error(int line, int column, const std::string& message) {
    if (!error_.has_value()) {
        SemanticError err;
        err.message = message;
        err.line = line;
        err.column = column;
        error_ = err;
    }
}

bool Analyzer::has_error() const {
    return error_.has_value();
}

std::string Analyzer::cell_key(int x, int y) {  // return the occupied cell coordinates as string
    return std::to_string(x) + ',' + std::to_string(y);
}

bool Analyzer::is_inside_world(int x, int y) const {  // the coordinates are inside the world is checked
    return x >= 0 && y >= 0 && x < world_width_ && y < world_height_;
}

void Analyzer::check_coordinate(int line, int column, int x, int y, const std::string& what) {
    if (!is_inside_world(x, y)) {
        std::ostringstream message;
        message << what << " position (" << x << ", " << y << ") is outside world ("
                << world_width_ << ", " << world_height_ << ')';
        report_error(line, column, message.str());
    }
}

// check cell is empty or not. Each cell can be uniquerly occupied by one entity, no duplicates allowed
void Analyzer::occupy_cell(int line, int column, int x, int y, const std::string& what) {
    const std::string key = cell_key(x, y);
    const auto existing = occupied_cells_.find(key);
    if (existing != occupied_cells_.end()) {
        std::ostringstream message;
        message << what << " at (" << x << ", " << y << ") overlaps another entity at ("
                << existing->second.first << ", " << existing->second.second << ')';
        report_error(line, column, message.str());
        return;
    }

    occupied_cells_[key] = {x, y};
}

void Analyzer::analyze_declaration(const ast::Decl& decl) {
    if (dynamic_cast<const ast::WorldDecl*>(&decl) != nullptr) {
        analyze_world(static_cast<const ast::WorldDecl&>(decl));
        return;
    }

    if (!has_world_) {
        report_error(decl.line, decl.column, "world must be declared before other declarations");
        return;
    }

    if (const auto* robot = dynamic_cast<const ast::RobotDecl*>(&decl)) {
        analyze_robot(*robot);
        return;
    }

    if (const auto* target = dynamic_cast<const ast::TargetDecl*>(&decl)) {
        analyze_target(*target);
        return;
    }

    if (const auto* obstacle = dynamic_cast<const ast::ObstacleDecl*>(&decl)) {
        analyze_obstacle(*obstacle);
        return;
    }

    if (const auto* behavior = dynamic_cast<const ast::BehaviorDecl*>(&decl)) {
        analyze_behavior(*behavior);
    }
}

void Analyzer::analyze_world(const ast::WorldDecl& world) {
    if (has_world_) {
        report_error(world.line, world.column, "program may declare only one world");
        return;
    }

    if (world.width <= 0 || world.height <= 0) {
        report_error(world.line, world.column, "world width and height must be positive");
        return;
    }

    has_world_ = true;
    world_width_ = world.width;
    world_height_ = world.height;
}


// each robot name is unique 
void Analyzer::analyze_robot(const ast::RobotDecl& robot) {
    if (robots_.contains(robot.name)) {
        report_error(robot.line, robot.column, "duplicate robot name '" + robot.name + "'");
        return;
    }

    check_coordinate(robot.line, robot.column, robot.x, robot.y, "robot '" + robot.name + "'");
    if (has_error()) {
        return;
    }

    occupy_cell(robot.line, robot.column, robot.x, robot.y, "robot '" + robot.name + "'");
    if (has_error()) {
        return;
    }

    robots_[robot.name] = {robot.x, robot.y};
}

// targets_ is a map of all target name to its coordinates
void Analyzer::analyze_target(const ast::TargetDecl& target) {
    if (targets_.contains(target.name)) {
        report_error(target.line, target.column, "duplicate target name '" + target.name + "'");
        return;
    }

    check_coordinate(target.line, target.column, target.x, target.y, "target '" + target.name + "'");
    if (has_error()) {
        return;
    }

    occupy_cell(target.line, target.column, target.x, target.y, "target '" + target.name + "'");
    if (has_error()) {
        return;
    }

    targets_[target.name] = {target.x, target.y};
}

void Analyzer::analyze_obstacle(const ast::ObstacleDecl& obstacle) {
    check_coordinate(obstacle.line, obstacle.column, obstacle.x, obstacle.y, "obstacle");
    if (has_error()) {
        return;
    }

    occupy_cell(obstacle.line, obstacle.column, obstacle.x, obstacle.y, "obstacle");
}

// check if the robot name is valid and not already has a behavior block
void Analyzer::analyze_behavior(const ast::BehaviorDecl& behavior) {

    // does the robot actually exist
    // Behavior name must match the robot name eg behaviour R 
    if (!robots_.contains(behavior.robot_name)) {
        report_error(behavior.line, behavior.column,
                     "behavior refers to unknown robot '" + behavior.robot_name + "'");
        return;
    }

    // check if the robot already has a behavior block existing
    if (robots_with_behavior_.contains(behavior.robot_name)) {
        report_error(behavior.line, behavior.column,
                     "robot '" + behavior.robot_name + "' already has a behavior block");
        return;
    }

    // if no
    // add the robot name to the robots_with_behavior_ map

    robots_with_behavior_.insert(behavior.robot_name);
    variables_.clear();
    in_behavior_ = true;

    for (const std::unique_ptr<ast::EveryTickStmt>& event : behavior.event_blocks) {
        analyze_every_tick(*event);
        if (has_error()) {
            break;
        }
    }

    in_behavior_ = false;
    variables_.clear();
}

void Analyzer::analyze_every_tick(const ast::EveryTickStmt& stmt) {
    if (stmt.body != nullptr) {
        analyze_block(*stmt.body);
    }
}

// semantically analyze the statements in 3 types: expression statement, block statement, if statement
void Analyzer::analyze_statement(const ast::Stmt& stmt) {
    if (const auto* expr_stmt = dynamic_cast<const ast::ExprStmt*>(&stmt)) {
        const ValueType type = analyze_expression(*expr_stmt->expression);
        if (!has_error()) {
            const bool is_assignment =
                dynamic_cast<const ast::AssignExpr*>(expr_stmt->expression.get()) != nullptr;
            if (type != ValueType::Void && !is_assignment) {
                report_error(expr_stmt->line, expr_stmt->column,
                             "expression statement must be an action call or assignment");
            }
        }
        return;
    }

    // block statement is a list of statements

    if (const auto* block = dynamic_cast<const ast::BlockStmt*>(&stmt)) {
        analyze_block(*block);
        return;
    }


    // if statement is a conditional statement

    if (const auto* if_stmt = dynamic_cast<const ast::IfStmt*>(&stmt)) {
        const ValueType condition_type = analyze_expression(*if_stmt->condition);
        if (!has_error() && condition_type != ValueType::Bool) {
            report_error(if_stmt->line, if_stmt->column, "if condition must be boolean");
            return;
        }

        analyze_statement(*if_stmt->then_branch);
        if (has_error()) {
            return;
        }

        if (if_stmt->else_branch != nullptr) {
            analyze_statement(*if_stmt->else_branch);
        }
    }
}

void Analyzer::analyze_block(const ast::BlockStmt& block) {
    for (const std::unique_ptr<ast::Stmt>& stmt : block.statements) {
        analyze_statement(*stmt);
        if (has_error()) {
            break;
        }
    }
}

// analyze the expression and return the type of the expression

ValueType Analyzer::analyze_expression(const ast::Expr& expr) {
    if (dynamic_cast<const ast::IntLiteralExpr*>(&expr) != nullptr) {
        return ValueType::Int;
    }

    if (dynamic_cast<const ast::BoolLiteralExpr*>(&expr) != nullptr) {
        return ValueType::Bool;
    }

    if (const auto* variable = dynamic_cast<const ast::VariableExpr*>(&expr)) {
        const auto found = variables_.find(variable->name);
        if (found == variables_.end()) {
            report_error(variable->line, variable->column,
                         "unknown variable '" + variable->name + "'");
            return ValueType::Int;
        }
        return found->second;
    }

    // check if a binary expression is valid by checking both operation and operands
    if (const auto* binary = dynamic_cast<const ast::BinaryExpr*>(&expr)) {
        const ValueType left = analyze_expression(*binary->left);
        if (has_error()) {
            return ValueType::Int;
        }

        const ValueType right = analyze_expression(*binary->right);
        if (has_error()) {
            return ValueType::Int;
        }

        return type_of_binary(binary->op, left, right, binary->line, binary->column);
    }

    // check if a unary expression is valid by checking the operand eg -5
    if (const auto* unary = dynamic_cast<const ast::UnaryExpr*>(&expr)) {
        const ValueType operand = analyze_expression(*unary->operand);
        if (has_error()) {
            return ValueType::Int;
        }

        return type_of_unary(unary->op, operand, unary->line, unary->column);
    }

    // check if a function call is valid by checking the function name and arguments in langauge rule
    if (const auto* call = dynamic_cast<const ast::CallExpr*>(&expr)) {
        return analyze_call(*call);
    }

    if (const auto* assign = dynamic_cast<const ast::AssignExpr*>(&expr)) {
        const ValueType value_type = analyze_expression(*assign->value);
        if (has_error()) {
            return ValueType::Int;
        }

        if (value_type != ValueType::Int && value_type != ValueType::Bool) {
            report_error(assign->line, assign->column, "assignment value must be integer or boolean");
            return ValueType::Int;
        }

        variables_[assign->name] = value_type;
        return value_type;
    }

    report_error(expr.line, expr.column, "unsupported expression");
    return ValueType::Int;
}

void Analyzer::check_target_argument(const ast::Expr& arg, const std::string& context) {
    const auto* variable = dynamic_cast<const ast::VariableExpr*>(&arg);
    if (variable == nullptr) {
        report_error(arg.line, arg.column, context + " requires a target name");
        return;
    }

    // check if the target name is valid (targets_ is a map of all target name to its coordinates)
    if (!targets_.contains(variable->name)) {
        report_error(variable->line, variable->column,
                     "unknown target '" + variable->name + "'");
    }
}

ValueType Analyzer::analyze_call(const ast::CallExpr& call) {
    if (!in_behavior_) {
        report_error(call.line, call.column, "built-in calls are only valid inside behavior blocks");
        return ValueType::Void;
    }

    const auto found = builtins().find(call.callee);
    if (found == builtins().end()) {
        report_error(call.line, call.column, "unknown function '" + call.callee + "'");
        return ValueType::Int;
    }

    const BuiltinInfo& info = found->second;
    if (static_cast<int>(call.arguments.size()) != info.arg_count) {
        std::ostringstream message;
        message << "function '" << call.callee << "' expects " << info.arg_count << " argument(s), got "
                << call.arguments.size();
        report_error(call.line, call.column, message.str());
        return info.return_type;
    }

    if (info.needs_target_arg) {
        check_target_argument(*call.arguments[0], "function '" + call.callee + "'");
        if (has_error()) {
            return info.return_type;
        }
    }

    return info.return_type;
}

ValueType Analyzer::type_of_binary(lexer::TokenType op, ValueType left, ValueType right, int line,
                                   int column) {
    switch (op) {
    case lexer::TokenType::Plus:
    case lexer::TokenType::Minus:
    case lexer::TokenType::Star:
    case lexer::TokenType::Slash:
        if (left != ValueType::Int || right != ValueType::Int) {
            report_error(line, column, "arithmetic operands must be integers");
            return ValueType::Int;
        }
        return ValueType::Int;

    case lexer::TokenType::Less:
    case lexer::TokenType::LessEqual:
    case lexer::TokenType::Greater:
    case lexer::TokenType::GreaterEqual:
        if (left != ValueType::Int || right != ValueType::Int) {
            report_error(line, column, "comparison operands must be integers");
            return ValueType::Int;
        }
        return ValueType::Bool;

    case lexer::TokenType::EqualEqual:
    case lexer::TokenType::BangEqual:
        if (left != right) {
            report_error(line, column, "equality operands must have the same type");
            return ValueType::Bool;
        }
        if (left != ValueType::Int && left != ValueType::Bool) {
            report_error(line, column, "equality operands must be integer or boolean");
            return ValueType::Bool;
        }
        return ValueType::Bool;

    default:
        report_error(line, column, "unsupported binary operator");
        return ValueType::Int;
    }
}

ValueType Analyzer::type_of_unary(lexer::TokenType op, ValueType operand, int line, int column) {
    switch (op) {
    case lexer::TokenType::Bang:
        if (operand != ValueType::Bool) {
            report_error(line, column, "logical not requires a boolean operand");
            return ValueType::Bool;
        }
        return ValueType::Bool;

    case lexer::TokenType::Minus:
        if (operand != ValueType::Int) {
            report_error(line, column, "unary minus requires an integer operand");
            return ValueType::Int;
        }
        return ValueType::Int;

    default:
        report_error(line, column, "unsupported unary operator");
        return ValueType::Int;
    }
}

} // namespace causis::semantic
