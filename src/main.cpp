#include <iostream>
#include <string>

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

Stage 0-1 complete; lexer (Stage 2) next.
)";

void print_help() {
    std::cout << kHelpText;
}

} // namespace

int main(int argc, char* argv[]) { // argc= no of commands, argv= array of commands
    if (argc == 1) {
        print_help();
        return 0;
    }

    const std::string command{argv[1]};
    if (command == "--help" || command == "-h" || command == "help") {
        print_help();
        return 0;
    }

    std::cerr << "causis: unknown or unavailable command '" << command << "'\n";
    std::cerr << "Run 'causis --help' to see available commands.\n";
    return 1;
}
