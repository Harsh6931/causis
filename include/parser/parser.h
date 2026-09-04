#pragma once

#include "ast/ast.h"
#include "lexer/token.h"

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace causis::parser {

struct ParseError {
    std::string message;
    int line{1};
    int column{1};
};

struct ParseResult {
    std::unique_ptr<ast::Program> program;
    std::optional<ParseError> error;
};

class Parser {
public:
    explicit Parser(std::vector<lexer::Token> tokens); // constructor takes tokens and store in member variable

    ParseResult parse_program();

private:
    std::vector<lexer::Token> tokens_;
    std::size_t current_{0};
    std::optional<ParseError> error_;

    bool is_at_end() const;
    const lexer::Token& peek() const;   // methods & entry point of the parser program
    const lexer::Token& previous() const;
    lexer::Token advance();
    bool check(lexer::TokenType type) const;
    bool match(lexer::TokenType type);
    lexer::Token consume(lexer::TokenType type, const std::string& message);

    void report_error(const std::string& message);

    //declarations
    
    std::unique_ptr<ast::Decl> parse_declaration();     
    std::unique_ptr<ast::WorldDecl> parse_world_decl();
    std::unique_ptr<ast::RobotDecl> parse_robot_decl();
    std::unique_ptr<ast::TargetDecl> parse_target_decl();
    std::unique_ptr<ast::ObstacleDecl> parse_obstacle_decl();
    std::unique_ptr<ast::BehaviorDecl> parse_behavior_decl();

    std::unique_ptr<ast::EveryTickStmt> parse_every_tick_block();
    std::unique_ptr<ast::Stmt> parse_statement();
    std::unique_ptr<ast::BlockStmt> parse_block();
    std::unique_ptr<ast::IfStmt> parse_if_statement();
    std::unique_ptr<ast::ExprStmt> parse_expression_statement();

    std::unique_ptr<ast::Expr> parse_expression();
    std::unique_ptr<ast::Expr> parse_assignment();
    std::unique_ptr<ast::Expr> parse_equality();
    std::unique_ptr<ast::Expr> parse_comparison();
    std::unique_ptr<ast::Expr> parse_term();
    std::unique_ptr<ast::Expr> parse_factor();
    std::unique_ptr<ast::Expr> parse_unary();
    std::unique_ptr<ast::Expr> parse_call();
    std::unique_ptr<ast::Expr> parse_primary();

    std::unique_ptr<ast::Expr> finish_call(std::unique_ptr<ast::Expr> callee_expr);
};

} // namespace causis::parser
