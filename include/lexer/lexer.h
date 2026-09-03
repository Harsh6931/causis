#pragma once

#include "lexer/token.h"

#include <optional>
#include <string>
#include <vector>

namespace causis::lexer {

struct LexerError {        // to store error message and position
    std::string message;
    int line{1};
    int column{1};
};

struct TokenizeResult {
    std::vector<Token> tokens;  // list of tokens
    std::optional<LexerError> error;
};

class Lexer {
public:
    explicit Lexer(std::string source);  //constructor to initialize the lexer with the source code

    Token next_token();          
    TokenizeResult tokenize(); // basically a loop of next_token()

private:
    std::string source_;        // all info stored of current token while scanning
    std::size_t position_{0};   // current position in the source code
    int line_{1};               // current line number
    int column_{1};             // current column number
    std::optional<LexerError> error_;  // optional error object to store error details

    bool is_at_end() const;
    char current() const;
    char peek_next() const;
    void advance();
    void skip_whitespace_and_comments();

    Token make_token(TokenType type, std::string lexeme, int token_line, int token_column);
    Token error_token(char bad_char);

    Token scan_identifier_or_keyword();    // scan to identify if its a identifier or keyword
    Token scan_integer();
    Token scan_operator_or_punctuation();
};

} // namespace causis::lexer
