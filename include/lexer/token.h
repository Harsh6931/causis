#pragma once

#include <optional>   // REMOVE IT IF NOT NEEDED
#include <string>

namespace causis::lexer {

enum class TokenType {
    // Keywords
    World,
    Robot,
    Target,
    Obstacle,
    Behavior,
    Every,
    Tick,
    If,
    Else,
    At,
    True,
    False,

    // Literals and names
    Integer,
    Identifier,

    // Punctuation
    LeftParen, // left paranthesis
    RightParen,
    LeftBrace,
    RightBrace,
    Semicolon,
    Comma,

    // Operators
    Equal,
    Plus,
    Minus,
    Star,
    Slash,
    EqualEqual,
    BangEqual,  // !=
    Less,
    LessEqual,
    Greater,
    GreaterEqual,
    Bang,  // !

    Error,
    EndOfFile,  // EOF
};

struct Token {  // AN OBJECT OF THIS CLASS IS TOKEN, I  WILL ALLOT IT TYPE AND LEXEME & POSIITON
    TokenType type{};   // kind of token
    std::string lexeme;   // which text produced it
    int line{1};     // where
    int column{1};
    int int_value{0};   // value of the token
};
//printing display helper i created
// i created for GoogleTest, not for Lexer Algorithm

// function declaration mapped to token.cpp
const char* token_type_name(TokenType type);  // give token name in Human readable format  eg. TokenType::Robot = Robot
std::string format_token(const Token& token); // shows entire token in readable format eg  Robot("ronny") at 1:7

} // namespace causis::lexer
