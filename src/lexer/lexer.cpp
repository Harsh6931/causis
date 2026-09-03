#include "lexer/lexer.h"

#include <cctype>
#include <unordered_map>

namespace causis::lexer {

namespace {

bool is_identifier_start(char ch) {
    return std::isalpha(static_cast<unsigned char>(ch)) || ch == '_';
}            // unsigned car used to avoid passing negative values

bool is_identifier_part(char ch) {
    return std::isalnum(static_cast<unsigned char>(ch)) || ch == '_';
}

bool is_digit_char(char ch) {
    return std::isdigit(static_cast<unsigned char>(ch));
}

TokenType keyword_type(const std::string& text) {
    static const std::unordered_map<std::string, TokenType> keywords = {
        {"world", TokenType::World},
        {"robot", TokenType::Robot},
        {"target", TokenType::Target},
        {"obstacle", TokenType::Obstacle},
        {"behavior", TokenType::Behavior},
        {"every", TokenType::Every},
        {"tick", TokenType::Tick},
        {"if", TokenType::If},
        {"else", TokenType::Else},
        {"at", TokenType::At},
        {"true", TokenType::True},
        {"false", TokenType::False},
    };

    const auto it = keywords.find(text);
    if (it != keywords.end()) {
        return it->second;
    }
    return TokenType::Identifier;
}       

// if not found in keyword table, then a identifier

} // namespace

Lexer::Lexer(std::string source) : source_(std::move(source)) {}

bool Lexer::is_at_end() const {
    return position_ >= source_.size();
}

char Lexer::current() const {
    if (is_at_end()) {
        return '\0';
    }
    return source_[position_];
}

char Lexer::peek_next() const {
    if (position_ + 1 >= source_.size()) {
        return '\0';
    }
    return source_[position_ + 1];
}

void Lexer::advance() {
    if (is_at_end()) {
        return;
    }

    if (current() == '\n') {
        line_++;
        column_ = 1;
    } else {
        column_++;
    }

    position_++;
}

void Lexer::skip_whitespace_and_comments() {
    while (!is_at_end()) {
        const char ch = current();

        if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n') {
            advance();
            continue;
        }

        if (ch == '/' && peek_next() == '/') {
            while (!is_at_end() && current() != '\n') {
                advance();
            }
            continue;
        }

        break;
    }
}

Token Lexer::make_token(TokenType type, std::string lexeme, int token_line, int token_column) {
    Token token;
    token.type = type;
    token.lexeme = std::move(lexeme);
    token.line = token_line;
    token.column = token_column;
    return token;
}

Token Lexer::error_token(char bad_char) {
    LexerError err;
    err.line = line_;
    err.column = column_;
    err.message = "unexpected character '";
    err.message += bad_char;
    err.message += "' at line ";
    err.message += std::to_string(err.line);
    err.message += ", column ";
    err.message += std::to_string(err.column);

    error_ = err;

    Token token = make_token(TokenType::Error, std::string(1, bad_char), err.line, err.column);
    advance();
    return token;
}

Token Lexer::scan_identifier_or_keyword() {
    const int token_line = line_;
    const int token_column = column_;

    std::string text;
    while (!is_at_end() && is_identifier_part(current())) {
        text += current();
        advance();
    }

    const TokenType type = keyword_type(text);
    return make_token(type, text, token_line, token_column);
}

Token Lexer::scan_integer() {
    const int token_line = line_;
    const int token_column = column_;

    std::string digits;
    while (!is_at_end() && is_digit_char(current())) {
        digits += current();
        advance();
    }

    Token token = make_token(TokenType::Integer, digits, token_line, token_column);
    token.int_value = std::stoi(digits);  // can error if too large just asying
    return token;
}

Token Lexer::scan_operator_or_punctuation() {
    const int token_line = line_;
    const int token_column = column_;
    const char ch = current();

    switch (ch) {
    case '(':
        advance();
        return make_token(TokenType::LeftParen, "(", token_line, token_column);
    case ')':
        advance();
        return make_token(TokenType::RightParen, ")", token_line, token_column);
    case '{':
        advance();
        return make_token(TokenType::LeftBrace, "{", token_line, token_column);
    case '}':
        advance();
        return make_token(TokenType::RightBrace, "}", token_line, token_column);
    case ';':
        advance();
        return make_token(TokenType::Semicolon, ";", token_line, token_column);
    case ',':
        advance();
        return make_token(TokenType::Comma, ",", token_line, token_column);
    case '+':
        advance();
        return make_token(TokenType::Plus, "+", token_line, token_column);
    case '-':
        advance();
        return make_token(TokenType::Minus, "-", token_line, token_column);
    case '*':
        advance();
        return make_token(TokenType::Star, "*", token_line, token_column);
    case '/':
        advance();
        return make_token(TokenType::Slash, "/", token_line, token_column);
    case '!':
        advance();
        if (current() == '=') {
            advance();
            return make_token(TokenType::BangEqual, "!=", token_line, token_column);
        }
        return make_token(TokenType::Bang, "!", token_line, token_column);
    case '=':
        advance();
        if (current() == '=') {
            advance();
            return make_token(TokenType::EqualEqual, "==", token_line, token_column);
        }
        return make_token(TokenType::Equal, "=", token_line, token_column);
    case '<':
        advance();
        if (current() == '=') {
            advance();
            return make_token(TokenType::LessEqual, "<=", token_line, token_column);
        }
        return make_token(TokenType::Less, "<", token_line, token_column);
    case '>':
        advance();
        if (current() == '=') {
            advance();
            return make_token(TokenType::GreaterEqual, ">=", token_line, token_column);
        }
        return make_token(TokenType::Greater, ">", token_line, token_column);
    default:
        return error_token(ch);
    }
}

Token Lexer::next_token() {
    if (error_) {
        return make_token(TokenType::EndOfFile, "", line_, column_);
    }

    skip_whitespace_and_comments();

    if (is_at_end()) {
        return make_token(TokenType::EndOfFile, "", line_, column_);
    }

    const char ch = current();

    if (is_identifier_start(ch)) {
        return scan_identifier_or_keyword();
    }

    if (is_digit_char(ch)) {
        return scan_integer();
    }

    return scan_operator_or_punctuation();
}

TokenizeResult Lexer::tokenize() {
    TokenizeResult result;

    while (true) {
        Token token = next_token();
        result.tokens.push_back(token);

        if (error_) {
            result.error = error_;
            break;
        }

        if (token.type == TokenType::Error || token.type == TokenType::EndOfFile) {
            break;
        }
    }

    return result;
}

} // namespace causis::lexer
