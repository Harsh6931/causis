#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "ast/ast.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "semantic/analyzer.h"

namespace {

constexpr const char* kHelpText = R"(causis - compiler and VM for 2D grid simulations

Usage:
  causis --help
  causis tokenize <program.ls>
  causis parse <program.ls>
  causis semantic <program.ls>
  causis ir <program.ls>
  causis optimize <program.ls>
  causis disassemble <program.ls>
  causis run <program.ls>

Stage 4 complete; simulation model (Stage 5) next.
)";

void print_help() {
    std::cout << kHelpText;
}

bool read_source_file(const std::string& path, std::string& source) {
    std::ifstream file(path);
    if (!file) {
        return false;
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    source = buffer.str();
    return true;
}

int run_tokenize(const std::string& path) {
    std::string source;
    if (!read_source_file(path, source)) {
        std::cerr << "causis: could not open file '" << path << "'\n";
        return 1;
    }

    causis::lexer::Lexer lexer(source);
    const causis::lexer::TokenizeResult result = lexer.tokenize();

    if (result.error.has_value()) {
        std::cerr << "Lexer error: " << result.error->message << '\n';
        return 1;
    }

    for (const causis::lexer::Token& token : result.tokens) {
        std::cout << causis::lexer::format_token(token) << '\n';
    }

    return 0;
}

int run_parse(const std::string& path) {
    std::string source;
    if (!read_source_file(path, source)) {
        std::cerr << "causis: could not open file '" << path << "'\n";
        return 1;
    }

    causis::lexer::Lexer lexer(source);
    const causis::lexer::TokenizeResult lex_result = lexer.tokenize();
    if (lex_result.error.has_value()) {
        std::cerr << "Lexer error: " << lex_result.error->message << '\n';
        return 1;
    }

    causis::parser::Parser parser(lex_result.tokens);
    const causis::parser::ParseResult parse_result = parser.parse_program();
    if (parse_result.error.has_value()) {
        std::cerr << "Parse error: " << parse_result.error->message << " at line "
                  << parse_result.error->line << ", column " << parse_result.error->column << '\n';
        return 1;
    }

    std::cout << causis::ast::print_program(*parse_result.program);
    return 0;
}

int run_semantic(const std::string& path) {
    std::string source;
    if (!read_source_file(path, source)) {
        std::cerr << "causis: could not open file '" << path << "'\n";
        return 1;
    }

    causis::lexer::Lexer lexer(source);
    const causis::lexer::TokenizeResult lex_result = lexer.tokenize();
    if (lex_result.error.has_value()) {
        std::cerr << "Lexer error: " << lex_result.error->message << '\n';
        return 1;
    }

    causis::parser::Parser parser(lex_result.tokens);
    const causis::parser::ParseResult parse_result = parser.parse_program();
    if (parse_result.error.has_value()) {
        std::cerr << "Parse error: " << parse_result.error->message << " at line "
                  << parse_result.error->line << ", column " << parse_result.error->column << '\n';
        return 1;
    }

    causis::semantic::Analyzer analyzer;
    const causis::semantic::SemanticResult semantic_result = analyzer.analyze(*parse_result.program);
    if (semantic_result.error.has_value()) {
        std::cerr << "Semantic error: " << semantic_result.error->message << " at line "
                  << semantic_result.error->line << ", column " << semantic_result.error->column
                  << '\n';
        return 1;
    }

    std::cout << "Semantic analysis passed.\n";
    return 0;
}

} // namespace

int main(int argc, char* argv[]) {
    if (argc == 1) {
        print_help();
        return 0;
    }

    const std::string command{argv[1]};
    if (command == "--help" || command == "-h" || command == "help") {
        print_help();
        return 0;
    }

    if (argc != 3) {
        std::cerr << "causis: usage: causis " << command << " <program.ls>\n";
        return 1;
    }

    if (command == "tokenize") {
        return run_tokenize(argv[2]);
    }

    if (command == "parse") {
        return run_parse(argv[2]);
    }

    if (command == "semantic") {
        return run_semantic(argv[2]);
    }

    std::cerr << "causis: unknown or unavailable command '" << command << "'\n";
    std::cerr << "Run 'causis --help' to see available commands.\n";
    return 1;
}
