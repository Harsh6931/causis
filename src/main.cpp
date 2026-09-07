#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "ast/ast.h"
#include "bytecode/compiler.h"
#include "bytecode/disassembler.h"
#include "cli/frontend.h"
#include "ir/lower.h"
#include "ir/optimizer.h"
#include "lexer/lexer.h"
#include "runtime/program_runner.h"

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
  causis run <program.ls> [--ticks N]

Default simulation length for run is 10 ticks.
)";

constexpr int kDefaultTicks = 10;

void print_help() {
  std::cout << kHelpText;
}

bool load_program_or_report(const std::string& path, std::unique_ptr<causis::ast::Program>& program) {
  causis::cli::FrontendResult loaded = causis::cli::load_program(path);
  if (!loaded.ok) {
    if (loaded.error.has_value()) {
      causis::cli::print_frontend_error(*loaded.error);
    } else {
      std::cerr << "Frontend error: failed to load program\n";
    }
    return false;
  }

  program = std::move(loaded.program);
  return true;
}

int run_tokenize(const std::string& path) {
  std::string source;
  std::ifstream file(path);
  if (!file) {
    std::cerr << "causis: could not open file '" << path << "'\n";
    return 1;
  }

  std::ostringstream buffer;
  buffer << file.rdbuf();
  source = buffer.str();

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
  const causis::cli::FrontendResult parsed = causis::cli::parse_program_file(path);
  if (!parsed.ok) {
    if (parsed.error.has_value()) {
      causis::cli::print_frontend_error(*parsed.error);
    }
    return 1;
  }

  std::cout << causis::ast::print_program(*parsed.program);
  return 0;
}

int run_semantic(const std::string& path) {
  std::unique_ptr<causis::ast::Program> program;
  if (!load_program_or_report(path, program)) {
    return 1;
  }

  std::cout << "Semantic analysis passed.\n";
  return 0;
}

int run_simulation(const std::string& path, int tick_count) {
  std::unique_ptr<causis::ast::Program> program;
  if (!load_program_or_report(path, program)) {
    return 1;
  }

  const causis::runtime::RunResult run_result = causis::runtime::run_program(*program, tick_count);
  if (run_result.error.has_value()) {
    std::cerr << "Runtime error: " << run_result.error->message << " at line "
              << run_result.error->line << ", column " << run_result.error->column << '\n';
    return 1;
  }

  if (!run_result.simulation.has_value()) {
    std::cerr << "Runtime error: simulation did not run\n";
    return 1;
  }

  std::cout << causis::runtime::format_run_summary(*run_result.simulation);
  return 0;
}

int run_ir(const std::string& path) {
  std::unique_ptr<causis::ast::Program> program;
  if (!load_program_or_report(path, program)) {
    return 1;
  }

  const causis::ir::LowerResult lower_result = causis::ir::lower_program(*program);
  if (lower_result.error.has_value()) {
    std::cerr << "IR lowering error: " << lower_result.error->message << " at line "
              << lower_result.error->line << ", column " << lower_result.error->column << '\n';
    return 1;
  }

  if (!lower_result.program.has_value()) {
    std::cerr << "IR lowering error: no program produced\n";
    return 1;
  }

  std::cout << causis::ir::print_ir(*lower_result.program);
  return 0;
}

int run_optimize(const std::string& path) {
  std::unique_ptr<causis::ast::Program> program;
  if (!load_program_or_report(path, program)) {
    return 1;
  }

  const causis::ir::LowerResult lower_result = causis::ir::lower_program(*program);
  if (lower_result.error.has_value()) {
    std::cerr << "IR lowering error: " << lower_result.error->message << " at line "
              << lower_result.error->line << ", column " << lower_result.error->column << '\n';
    return 1;
  }

  if (!lower_result.program.has_value()) {
    std::cerr << "IR lowering error: no program produced\n";
    return 1;
  }

  std::cout << "=== IR (before) ===\n";
  std::cout << causis::ir::print_ir(*lower_result.program);
  std::cout << "=== IR (optimized) ===\n";
  std::cout << causis::ir::print_ir(causis::ir::optimize_program(*lower_result.program));
  return 0;
}

int run_disassemble(const std::string& path) {
  std::unique_ptr<causis::ast::Program> program;
  if (!load_program_or_report(path, program)) {
    return 1;
  }

  const causis::ir::LowerResult lower_result = causis::ir::lower_program(*program);
  if (lower_result.error.has_value()) {
    std::cerr << "IR lowering error: " << lower_result.error->message << " at line "
              << lower_result.error->line << ", column " << lower_result.error->column << '\n';
    return 1;
  }

  if (!lower_result.program.has_value()) {
    std::cerr << "IR lowering error: no program produced\n";
    return 1;
  }

  const causis::ir::IrProgram optimized = causis::ir::optimize_program(*lower_result.program);
  const causis::bytecode::CompileResult compile_result = causis::bytecode::compile_ir(optimized);
  if (compile_result.error.has_value()) {
    std::cerr << "Bytecode compile error: " << compile_result.error->message << " at line "
              << compile_result.error->line << ", column " << compile_result.error->column << '\n';
    return 1;
  }

  if (!compile_result.program.has_value()) {
    std::cerr << "Bytecode compile error: no program produced\n";
    return 1;
  }

  std::cout << causis::bytecode::disassemble_program(*compile_result.program);
  return 0;
}

bool parse_tick_count(const char* text, int& tick_count) {
  try {
    std::size_t consumed = 0;
    const long long value = std::stoll(text, &consumed);
    if (consumed == 0 || text[consumed] != '\0' || value < 0) {
      return false;
    }
    tick_count = static_cast<int>(value);
    return true;
  } catch (const std::exception&) {
    return false;
  }
}

bool parse_run_command(int argc, char* argv[], std::string& path, int& tick_count) {
  path = argv[2];
  tick_count = kDefaultTicks;

  if (argc == 3) {
    return true;
  }

  if (argc == 5 && std::string(argv[3]) == "--ticks") {
    if (!parse_tick_count(argv[4], tick_count)) {
      std::cerr << "causis: --ticks requires a non-negative integer\n";
      return false;
    }
    return true;
  }

  std::cerr << "causis: usage: causis run <program.ls> [--ticks N]\n";
  return false;
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

  if (command == "run") {
    if (argc < 3 || argc == 4 || argc > 5) {
      std::cerr << "causis: usage: causis run <program.ls> [--ticks N]\n";
      return 1;
    }

    std::string path;
    int tick_count = kDefaultTicks;
    if (!parse_run_command(argc, argv, path, tick_count)) {
      return 1;
    }

    return run_simulation(path, tick_count);
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

  if (command == "ir") {
    return run_ir(argv[2]);
  }

  if (command == "optimize") {
    return run_optimize(argv[2]);
  }

  if (command == "disassemble") {
    return run_disassemble(argv[2]);
  }

  std::cerr << "causis: unknown or unavailable command '" << command << "'\n";
  std::cerr << "Run 'causis --help' to see available commands.\n";
  return 1;
}
