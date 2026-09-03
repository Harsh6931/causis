#include "lexer/token.h"

namespace causis::lexer {

    // useful when token dump CLI
const char* token_type_name(TokenType type) {
    switch (type) {
    case TokenType::World:
        return "WORLD";
    case TokenType::Robot:
        return "ROBOT";
    case TokenType::Target:
        return "TARGET";
    case TokenType::Obstacle:
        return "OBSTACLE";
    case TokenType::Behavior:
        return "BEHAVIOR";
    case TokenType::Every:
        return "EVERY";
    case TokenType::Tick:
        return "TICK";
    case TokenType::If:
        return "IF";
    case TokenType::Else:
        return "ELSE";
    case TokenType::At:
        return "AT";
    case TokenType::True:
        return "TRUE";
    case TokenType::False:
        return "FALSE";
    case TokenType::Integer:
        return "INTEGER";
    case TokenType::Identifier:
        return "IDENTIFIER";
    case TokenType::LeftParen:
        return "LEFT_PAREN";
    case TokenType::RightParen:
        return "RIGHT_PAREN";
    case TokenType::LeftBrace:
        return "LEFT_BRACE";
    case TokenType::RightBrace:
        return "RIGHT_BRACE";
    case TokenType::Semicolon:
        return "SEMICOLON";
    case TokenType::Comma:
        return "COMMA";
    case TokenType::Equal:
        return "EQUAL";
    case TokenType::Plus:
        return "PLUS";
    case TokenType::Minus:
        return "MINUS";
    case TokenType::Star:
        return "STAR";
    case TokenType::Slash:
        return "SLASH";
    case TokenType::EqualEqual:
        return "EQUAL_EQUAL";
    case TokenType::BangEqual:
        return "BANG_EQUAL";
    case TokenType::Less:
        return "LESS";
    case TokenType::LessEqual:
        return "LESS_EQUAL";
    case TokenType::Greater:
        return "GREATER";
    case TokenType::GreaterEqual:
        return "GREATER_EQUAL";
    case TokenType::Bang:
        return "BANG";
    case TokenType::Error:
        return "ERROR";
    case TokenType::EndOfFile:
        return "END_OF_FILE";
    }
    return "UNKNOWN";
}

std::string format_token(const Token& token) {
    const char* name = token_type_name(token.type);

    if (token.type == TokenType::Integer) {
        return std::string(name) + "(" + std::to_string(token.int_value) + ")";
    }

    // eg = IDENTIFIER("ronny")         here not adding at + position as not needed and i can access it as (token.line)

    if (token.type == TokenType::Identifier) {
        return std::string(name) + "(" + token.lexeme + ")";
    }

    return name;
}

} // namespace causis::lexer


// this file is for implementing utilities of token representation