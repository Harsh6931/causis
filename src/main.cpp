#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "lexer/lexer.h"

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

Stage 2 complete; parser (Stage 3) next.
)";

void print_help() {
    std::cout << kHelpText;
}

int run_tokenize(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        std::cerr << "causis: could not open file '" << path << "'\n";
        return 1;
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    const std::string source = buffer.str();

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

    if (command == "tokenize") {
        if (argc != 3) {
            std::cerr << "causis: usage: causis tokenize <program.ls>\n";
            return 1;
        }
        return run_tokenize(argv[2]);
    }

    std::cerr << "causis: unknown or unavailable command '" << command << "'\n";
    std::cerr << "Run 'causis --help' to see available commands.\n";
    return 1;
}
