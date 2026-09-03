#include "lexer/lexer.h"

#include <fstream>
#include <sstream>

#include <algorithm>
#include <gtest/gtest.h>

namespace causis::lexer {
namespace {

std::string read_file(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        return {};
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

TokenizeResult lex(const std::string& source) {
    Lexer lexer(source);
    return lexer.tokenize();
}

std::vector<TokenType> token_types(const TokenizeResult& result) {
    std::vector<TokenType> types;
    for (const Token& token : result.tokens) {
        types.push_back(token.type);
    }
    return types;
}

} // namespace

TEST(LexerKeywords, RecognizesReservedWords) {
    const TokenizeResult result = lex("world robot if else true false at every tick");

    ASSERT_FALSE(result.error.has_value());
    ASSERT_EQ(result.tokens.size(), 10U);

    EXPECT_EQ(result.tokens[0].type, TokenType::World);
    EXPECT_EQ(result.tokens[1].type, TokenType::Robot);
    EXPECT_EQ(result.tokens[2].type, TokenType::If);
    EXPECT_EQ(result.tokens[3].type, TokenType::Else);
    EXPECT_EQ(result.tokens[4].type, TokenType::True);
    EXPECT_EQ(result.tokens[5].type, TokenType::False);
    EXPECT_EQ(result.tokens[6].type, TokenType::At);
    EXPECT_EQ(result.tokens[7].type, TokenType::Every);
    EXPECT_EQ(result.tokens[8].type, TokenType::Tick);
    EXPECT_EQ(result.tokens[9].type, TokenType::EndOfFile);
}

TEST(LexerIdentifiers, TreatsNonKeywordsAsIdentifiers) {
    const TokenizeResult result = lex("R speed move_right");

    ASSERT_FALSE(result.error.has_value());
    ASSERT_EQ(result.tokens.size(), 4U);

    EXPECT_EQ(result.tokens[0].type, TokenType::Identifier);
    EXPECT_EQ(result.tokens[0].lexeme, "R");
    EXPECT_EQ(result.tokens[1].type, TokenType::Identifier);
    EXPECT_EQ(result.tokens[1].lexeme, "speed");
    EXPECT_EQ(result.tokens[2].type, TokenType::Identifier);
    EXPECT_EQ(result.tokens[2].lexeme, "move_right");
}

TEST(LexerIntegers, ReadsIntegerLiterals) {
    const TokenizeResult result = lex("0 42 100");

    ASSERT_FALSE(result.error.has_value());
    ASSERT_EQ(result.tokens.size(), 4U);

    EXPECT_EQ(result.tokens[0].type, TokenType::Integer);
    EXPECT_EQ(result.tokens[0].int_value, 0);
    EXPECT_EQ(result.tokens[1].int_value, 42);
    EXPECT_EQ(result.tokens[2].int_value, 100);
}

TEST(LexerOperators, ReadsMultiCharOperators) {
    const TokenizeResult result = lex("== != <= >= + -");

    ASSERT_FALSE(result.error.has_value());

    const std::vector<TokenType> types = token_types(result);
    const std::vector<TokenType> expected = {
        TokenType::EqualEqual,
        TokenType::BangEqual,
        TokenType::LessEqual,
        TokenType::GreaterEqual,
        TokenType::Plus,
        TokenType::Minus,
        TokenType::EndOfFile,
    };

    EXPECT_EQ(types, expected);
}

TEST(LexerComments, SkipsLineComments) {
    const TokenizeResult result = lex("robot R; // comment\nat");

    ASSERT_FALSE(result.error.has_value());

    const std::vector<TokenType> types = token_types(result);
    const std::vector<TokenType> expected = {
        TokenType::Robot,
        TokenType::Identifier,
        TokenType::Semicolon,
        TokenType::At,
        TokenType::EndOfFile,
    };

    EXPECT_EQ(types, expected);
}

TEST(LexerErrors, RejectsInvalidCharacters) {
    const TokenizeResult quote_error = lex("\"hello");
    ASSERT_TRUE(quote_error.error.has_value());
    ASSERT_EQ(quote_error.tokens.back().type, TokenType::Error);
    EXPECT_NE(quote_error.error->message.find("line"), std::string::npos);
    EXPECT_NE(quote_error.error->message.find("column"), std::string::npos);

    const TokenizeResult float_error = lex("3.14");
    ASSERT_TRUE(float_error.error.has_value());
    ASSERT_EQ(float_error.tokens.back().type, TokenType::Error);
    EXPECT_EQ(float_error.error->line, 1);
    EXPECT_EQ(float_error.error->column, 2);
}

TEST(LexerExamples, TokenizesBasicMoveFile) {
    const std::string source = read_file("examples/basic_move.ls");
    ASSERT_FALSE(source.empty());

    const TokenizeResult result = lex(source);
    ASSERT_FALSE(result.error.has_value());

    const std::vector<TokenType> types = token_types(result);
    const std::vector<TokenType> expected = {
        TokenType::World,
        TokenType::Integer,
        TokenType::Integer,
        TokenType::Semicolon,
        TokenType::Robot,
        TokenType::Identifier,
        TokenType::At,
        TokenType::Integer,
        TokenType::Integer,
        TokenType::Semicolon,
        TokenType::Behavior,
        TokenType::Identifier,
        TokenType::LeftBrace,
        TokenType::Every,
        TokenType::Tick,
        TokenType::LeftBrace,
        TokenType::Identifier,
        TokenType::LeftParen,
        TokenType::RightParen,
        TokenType::Semicolon,
        TokenType::RightBrace,
        TokenType::RightBrace,
        TokenType::EndOfFile,
    };

    EXPECT_EQ(types, expected);
}

TEST(LexerExamples, TokenizesCollisionFile) {
    const std::string source = read_file("examples/collision.ls");
    ASSERT_FALSE(source.empty());

    const TokenizeResult result = lex(source);
    ASSERT_FALSE(result.error.has_value());

    const std::vector<TokenType> types = token_types(result);

    EXPECT_EQ(types[0], TokenType::World);
    EXPECT_NE(std::find(types.begin(), types.end(), TokenType::If), types.end());
    EXPECT_NE(std::find(types.begin(), types.end(), TokenType::Else), types.end());
    EXPECT_EQ(types.back(), TokenType::EndOfFile);
}

TEST(LexerExamples, TokenizesTargetFile) {
    const std::string source = read_file("examples/target.ls");
    ASSERT_FALSE(source.empty());

    const TokenizeResult result = lex(source);
    ASSERT_FALSE(result.error.has_value());

    const std::vector<TokenType> types = token_types(result);

    EXPECT_EQ(types[0], TokenType::World);
    EXPECT_NE(std::find(types.begin(), types.end(), TokenType::Target), types.end());
    EXPECT_EQ(types.back(), TokenType::EndOfFile);
}

} // namespace causis::lexer
