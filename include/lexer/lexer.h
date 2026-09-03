#pragma once

#include "lexer/token.h"

#include <optional>
#include <string>
#include <vector>

namespace causis::lexer {

struct LexerError {
    std::string message;
    int line{1};
    int column{1};
};

struct TokenizeResult {
    std::vector<Token> tokens;
    std::optional<LexerError> error;
};

class Lexer {
public:
    explicit Lexer(std::string source);

    Token next_token();
    TokenizeResult tokenize();

private:
    std::string source_;
    std::size_t position_{0};
    int line_{1};
    int column_{1};
    bool has_error_{false};
    std::optional<LexerError> error_;

    bool is_at_end() const;
    char current() const;
    char peek_next() const;
    void advance();
    void skip_whitespace_and_comments();

    Token make_token(TokenType type, std::string lexeme, int token_line, int token_column);
    Token error_token(char bad_char);

    Token scan_identifier_or_keyword();
    Token scan_integer();
    Token scan_operator_or_punctuation();
};

} // namespace causis::lexer
