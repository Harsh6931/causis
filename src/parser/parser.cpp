// This is to parse the AST and convert to human readable text

#include "parser/parser.h"

namespace causis::parser {

Parser::Parser(std::vector<lexer::Token> tokens) : tokens_(std::move(tokens)) {}

bool Parser::is_at_end() const {
    return peek().type == lexer::TokenType::EndOfFile;
}

const lexer::Token& Parser::peek() const {
    return tokens_[current_];
}

const lexer::Token& Parser::previous() const {
    return tokens_[current_ - 1];
}

lexer::Token Parser::advance() {
    if (!is_at_end()) {
        current_++;
    }
    return previous();
}

bool Parser::check(lexer::TokenType type) const {
    if (is_at_end()) {
        return false;
    }
    return peek().type == type;
}

bool Parser::match(lexer::TokenType type) {
    if (!check(type)) {
        return false;
    }
    advance();
    return true;
}

lexer::Token Parser::consume(lexer::TokenType type, const std::string& message) {
    if (check(type)) {
        return advance();
    }

    report_error(message);
    return peek();
}

void Parser::report_error(const std::string& message) {
    if (!error_) {
        ParseError err;
        err.message = message;
        err.line = peek().line;
        err.column = peek().column;
        error_ = err;
    }
}

ParseResult Parser::parse_program() {
    ParseResult result;
    auto program = std::make_unique<ast::Program>();

    while (!is_at_end() && !error_) {
        program->declarations.push_back(parse_declaration());
    }

    if (error_) {
        result.error = error_;
        return result;
    }

    result.program = std::move(program);
    return result;
}

std::unique_ptr<ast::Decl> Parser::parse_declaration() {
    if (match(lexer::TokenType::World)) {
        return parse_world_decl();
    }
    if (match(lexer::TokenType::Robot)) {
        return parse_robot_decl();
    }
    if (match(lexer::TokenType::Target)) {
        return parse_target_decl();
    }
    if (match(lexer::TokenType::Obstacle)) {
        return parse_obstacle_decl();
    }
    if (match(lexer::TokenType::Behavior)) {
        return parse_behavior_decl();
    }

    report_error("expected a top-level declaration");
    return nullptr;
}

std::unique_ptr<ast::WorldDecl> Parser::parse_world_decl() {
    auto decl = std::make_unique<ast::WorldDecl>();
    decl->line = previous().line;
    decl->column = previous().column;

    const lexer::Token width_token = consume(lexer::TokenType::Integer, "expected world width");
    decl->width = width_token.int_value;

    const lexer::Token height_token = consume(lexer::TokenType::Integer, "expected world height");
    decl->height = height_token.int_value;

    consume(lexer::TokenType::Semicolon, "expected ';' after world declaration");
    return decl;
}

std::unique_ptr<ast::RobotDecl> Parser::parse_robot_decl() {
    auto decl = std::make_unique<ast::RobotDecl>();
    decl->line = previous().line;
    decl->column = previous().column;

    const lexer::Token name_token = consume(lexer::TokenType::Identifier, "expected robot name");
    decl->name = name_token.lexeme;

    consume(lexer::TokenType::At, "expected 'at' after robot name");

    const lexer::Token x_token = consume(lexer::TokenType::Integer, "expected robot x coordinate");
    decl->x = x_token.int_value;

    const lexer::Token y_token = consume(lexer::TokenType::Integer, "expected robot y coordinate");
    decl->y = y_token.int_value;

    consume(lexer::TokenType::Semicolon, "expected ';' after robot declaration");
    return decl;
}

std::unique_ptr<ast::TargetDecl> Parser::parse_target_decl() {
    auto decl = std::make_unique<ast::TargetDecl>();
    decl->line = previous().line;
    decl->column = previous().column;

    const lexer::Token name_token = consume(lexer::TokenType::Identifier, "expected target name");
    decl->name = name_token.lexeme;

    consume(lexer::TokenType::At, "expected 'at' after target name");

    const lexer::Token x_token = consume(lexer::TokenType::Integer, "expected target x coordinate");
    decl->x = x_token.int_value;

    const lexer::Token y_token = consume(lexer::TokenType::Integer, "expected target y coordinate");
    decl->y = y_token.int_value;

    consume(lexer::TokenType::Semicolon, "expected ';' after target declaration");
    return decl;
}

std::unique_ptr<ast::ObstacleDecl> Parser::parse_obstacle_decl() {
    auto decl = std::make_unique<ast::ObstacleDecl>();
    decl->line = previous().line;
    decl->column = previous().column;

    consume(lexer::TokenType::At, "expected 'at' after obstacle");

    const lexer::Token x_token = consume(lexer::TokenType::Integer, "expected obstacle x coordinate");
    decl->x = x_token.int_value;

    const lexer::Token y_token = consume(lexer::TokenType::Integer, "expected obstacle y coordinate");
    decl->y = y_token.int_value;

    consume(lexer::TokenType::Semicolon, "expected ';' after obstacle declaration");
    return decl;
}

std::unique_ptr<ast::BehaviorDecl> Parser::parse_behavior_decl() {
    auto decl = std::make_unique<ast::BehaviorDecl>();
    decl->line = previous().line;
    decl->column = previous().column;

    const lexer::Token name_token = consume(lexer::TokenType::Identifier, "expected behavior robot name");
    decl->robot_name = name_token.lexeme;

    consume(lexer::TokenType::LeftBrace, "expected '{' after behavior name");

    while (!check(lexer::TokenType::RightBrace) && !is_at_end() && !error_) {
        decl->event_blocks.push_back(parse_every_tick_block());
    }

    consume(lexer::TokenType::RightBrace, "expected '}' after behavior block");
    return decl;
}

std::unique_ptr<ast::EveryTickStmt> Parser::parse_every_tick_block() {
    auto stmt = std::make_unique<ast::EveryTickStmt>();
    consume(lexer::TokenType::Every, "expected 'every'");
    stmt->line = previous().line;
    stmt->column = previous().column;
    consume(lexer::TokenType::Tick, "expected 'tick'");
    stmt->body = parse_block();
    return stmt;
}

std::unique_ptr<ast::Stmt> Parser::parse_statement() {
    if (match(lexer::TokenType::If)) {
        return parse_if_statement();
    }
    if (check(lexer::TokenType::LeftBrace)) {
        return parse_block();
    }
    return parse_expression_statement();
}

std::unique_ptr<ast::BlockStmt> Parser::parse_block() {
    auto block = std::make_unique<ast::BlockStmt>();
    consume(lexer::TokenType::LeftBrace, "expected '{'");

    while (!check(lexer::TokenType::RightBrace) && !is_at_end() && !error_) {
        block->statements.push_back(parse_statement());
    }

    consume(lexer::TokenType::RightBrace, "expected '}'");
    return block;
}

std::unique_ptr<ast::IfStmt> Parser::parse_if_statement() {
    auto stmt = std::make_unique<ast::IfStmt>();
    stmt->line = previous().line;
    stmt->column = previous().column;

    stmt->condition = parse_expression();

    if (check(lexer::TokenType::LeftBrace)) {
        stmt->then_branch = parse_block();
    } else {
        report_error("expected block after if condition");
    }

    if (match(lexer::TokenType::Else)) {
        if (check(lexer::TokenType::LeftBrace)) {
            stmt->else_branch = parse_block();
        } else {
            report_error("expected block after else");
        }
    }

    return stmt;
}

std::unique_ptr<ast::ExprStmt> Parser::parse_expression_statement() {
    auto stmt = std::make_unique<ast::ExprStmt>();
    stmt->expression = parse_expression();
    stmt->line = stmt->expression->line;
    stmt->column = stmt->expression->column;
    consume(lexer::TokenType::Semicolon, "expected ';' after expression");
    return stmt;
}

std::unique_ptr<ast::Expr> Parser::parse_expression() {
    return parse_assignment();
}

std::unique_ptr<ast::Expr> Parser::parse_assignment() {
    std::unique_ptr<ast::Expr> expr = parse_equality();

    if (auto* variable = dynamic_cast<ast::VariableExpr*>(expr.get())) {
        if (match(lexer::TokenType::Equal)) {
            auto assign = std::make_unique<ast::AssignExpr>();
            assign->line = variable->line;
            assign->column = variable->column;
            assign->name = variable->name;
            assign->value = parse_assignment();
            return assign;
        }
    }

    return expr;
}

std::unique_ptr<ast::Expr> Parser::parse_equality() {
    std::unique_ptr<ast::Expr> expr = parse_comparison();

    while (match(lexer::TokenType::EqualEqual) || match(lexer::TokenType::BangEqual)) {
        const lexer::Token op = previous();
        auto binary = std::make_unique<ast::BinaryExpr>();
        binary->line = op.line;
        binary->column = op.column;
        binary->op = op.type;
        binary->left = std::move(expr);
        binary->right = parse_comparison();
        expr = std::move(binary);
    }

    return expr;
}

std::unique_ptr<ast::Expr> Parser::parse_comparison() {
    std::unique_ptr<ast::Expr> expr = parse_term();

    while (match(lexer::TokenType::Less) || match(lexer::TokenType::LessEqual) ||
           match(lexer::TokenType::Greater) || match(lexer::TokenType::GreaterEqual)) {
        const lexer::Token op = previous();
        auto binary = std::make_unique<ast::BinaryExpr>();
        binary->line = op.line;
        binary->column = op.column;
        binary->op = op.type;
        binary->left = std::move(expr);
        binary->right = parse_term();
        expr = std::move(binary);
    }

    return expr;
}

std::unique_ptr<ast::Expr> Parser::parse_term() {
    std::unique_ptr<ast::Expr> expr = parse_factor();

    while (match(lexer::TokenType::Plus) || match(lexer::TokenType::Minus)) {
        const lexer::Token op = previous();
        auto binary = std::make_unique<ast::BinaryExpr>();
        binary->line = op.line;
        binary->column = op.column;
        binary->op = op.type;
        binary->left = std::move(expr);
        binary->right = parse_factor();
        expr = std::move(binary);
    }

    return expr;
}

std::unique_ptr<ast::Expr> Parser::parse_factor() {
    std::unique_ptr<ast::Expr> expr = parse_unary();

    while (match(lexer::TokenType::Star) || match(lexer::TokenType::Slash)) {
        const lexer::Token op = previous();
        auto binary = std::make_unique<ast::BinaryExpr>();
        binary->line = op.line;
        binary->column = op.column;
        binary->op = op.type;
        binary->left = std::move(expr);
        binary->right = parse_unary();
        expr = std::move(binary);
    }

    return expr;
}

std::unique_ptr<ast::Expr> Parser::parse_unary() {
    if (match(lexer::TokenType::Bang) || match(lexer::TokenType::Minus)) {
        const lexer::Token op = previous();
        auto unary = std::make_unique<ast::UnaryExpr>();
        unary->line = op.line;
        unary->column = op.column;
        unary->op = op.type;
        unary->operand = parse_unary();
        return unary;
    }

    return parse_call();
}

std::unique_ptr<ast::Expr> Parser::parse_call() {
    std::unique_ptr<ast::Expr> expr = parse_primary();

    while (match(lexer::TokenType::LeftParen)) {
        expr = finish_call(std::move(expr));
    }

    return expr;
}

std::unique_ptr<ast::Expr> Parser::finish_call(std::unique_ptr<ast::Expr> callee_expr) {
    auto call = std::make_unique<ast::CallExpr>();

    if (auto* variable = dynamic_cast<ast::VariableExpr*>(callee_expr.get())) {
        call->line = variable->line;
        call->column = variable->column;
        call->callee = variable->name;
    } else {
        report_error("expected function name before '('");
        return callee_expr;
    }

    if (!check(lexer::TokenType::RightParen)) {
        do {
            call->arguments.push_back(parse_expression());
        } while (match(lexer::TokenType::Comma));
    }

    consume(lexer::TokenType::RightParen, "expected ')' after arguments");
    return call;
}

std::unique_ptr<ast::Expr> Parser::parse_primary() {
    if (match(lexer::TokenType::Integer)) {
        auto expr = std::make_unique<ast::IntLiteralExpr>();
        expr->line = previous().line;
        expr->column = previous().column;
        expr->value = previous().int_value;
        return expr;
    }

    if (match(lexer::TokenType::True)) {
        auto expr = std::make_unique<ast::BoolLiteralExpr>();
        expr->line = previous().line;
        expr->column = previous().column;
        expr->value = true;
        return expr;
    }

    if (match(lexer::TokenType::False)) {
        auto expr = std::make_unique<ast::BoolLiteralExpr>();
        expr->line = previous().line;
        expr->column = previous().column;
        expr->value = false;
        return expr;
    }

    if (match(lexer::TokenType::Identifier)) {
        auto expr = std::make_unique<ast::VariableExpr>();
        expr->line = previous().line;
        expr->column = previous().column;
        expr->name = previous().lexeme;
        return expr;
    }

    if (match(lexer::TokenType::LeftParen)) {
        std::unique_ptr<ast::Expr> expr = parse_expression();
        consume(lexer::TokenType::RightParen, "expected ')' after expression");
        return expr;
    }

    report_error("expected expression");
    return std::make_unique<ast::IntLiteralExpr>();
}

} // namespace causis::parser
