#pragma once

#include "ast/ast.h"

#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace causis::semantic {

enum class ValueType {
    Int,
    Bool,
    Void,
};

struct SemanticError {
    std::string message;
    int line{1};
    int column{1};
};

struct SemanticResult {  //either grammar is correct or not
    bool ok{false};
    std::optional<SemanticError> error;
};

class Analyzer {
public:
    SemanticResult analyze(const ast::Program& program);

private:
    bool has_world_{false}; //world is required
    int world_width_{0};
    int world_height_{0};

    std::unordered_map<std::string, std::pair<int, int>> robots_;
    std::unordered_map<std::string, std::pair<int, int>> targets_;
    std::unordered_map<std::string, ValueType> variables_;
    std::unordered_map<std::string, std::pair<int, int>> occupied_cells_;
    std::unordered_set<std::string> robots_with_behavior_;

    bool in_behavior_{false};
    std::optional<SemanticError> error_;

    void report_error(int line, int column, const std::string& message);
    bool has_error() const;

    static std::string cell_key(int x, int y);

    bool is_inside_world(int x, int y) const;
    void check_coordinate(int line, int column, int x, int y, const std::string& what);
    void occupy_cell(int line, int column, int x, int y, const std::string& what);

    void analyze_declaration(const ast::Decl& decl);
    void analyze_world(const ast::WorldDecl& world);
    void analyze_robot(const ast::RobotDecl& robot);
    void analyze_target(const ast::TargetDecl& target);
    void analyze_obstacle(const ast::ObstacleDecl& obstacle);
    void analyze_behavior(const ast::BehaviorDecl& behavior);

    void analyze_every_tick(const ast::EveryTickStmt& stmt);
    void analyze_statement(const ast::Stmt& stmt);
    void analyze_block(const ast::BlockStmt& block);

    ValueType analyze_expression(const ast::Expr& expr);
    ValueType analyze_call(const ast::CallExpr& call);
    void check_target_argument(const ast::Expr& arg, const std::string& context);

    ValueType type_of_binary(lexer::TokenType op, ValueType left, ValueType right, int line, int column);
    ValueType type_of_unary(lexer::TokenType op, ValueType operand, int line, int column);
};

} // namespace causis::semantic
