#include <fstream>
#include <iostream>
#include <optional>
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
#include "runtime/tick_log.h"

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
  causis run <program.ls> [--ticks N] [--log tick-log.json]

Default simulation length for run is 10 ticks.
Use --log to write a tick-by-tick JSON log for visualizer/index.html.
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

struct RunOptions {
  std::string path;
  int tick_count{kDefaultTicks};
  std::optional<std::string> log_path;
};

int run_simulation(const RunOptions& options) {
  std::unique_ptr<causis::ast::Program> program;
  if (!load_program_or_report(options.path, program)) {
    return 1;
  }

  causis::runtime::TickLog tick_log;
  causis::runtime::TickLog* log_ptr = options.log_path.has_value() ? &tick_log : nullptr;

  const causis::runtime::RunResult run_result =
      causis::runtime::run_program(*program, options.tick_count, log_ptr);
  if (run_result.error.has_value()) {
    std::cerr << "Runtime error: " << run_result.error->message << " at line "
              << run_result.error->line << ", column " << run_result.error->column << '\n';
    return 1;
  }

  if (!run_result.simulation.has_value()) {
    std::cerr << "Runtime error: simulation did not run\n";
    return 1;
  }

  if (options.log_path.has_value()) {
    if (!causis::runtime::write_tick_log_file(*options.log_path, tick_log)) {
      std::cerr << "causis: could not write tick log '" << *options.log_path << "'\n";
      return 1;
    }
    std::cout << "Wrote tick log to " << *options.log_path << ".\n";
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

bool parse_run_command(int argc, char* argv[], RunOptions& options) {
  options.path = argv[2];
  options.tick_count = kDefaultTicks;
  options.log_path.reset();

  for (int i = 3; i < argc; ++i) {
    const std::string flag{argv[i]};
    if (flag == "--ticks") {
      if (i + 1 >= argc) {
        std::cerr << "causis: --ticks requires a value\n";
        return false;
      }
      if (!parse_tick_count(argv[i + 1], options.tick_count)) {
        std::cerr << "causis: --ticks requires a non-negative integer\n";
        return false;
      }
      ++i;
      continue;
    }

    if (flag == "--log") {
      if (i + 1 >= argc) {
        std::cerr << "causis: --log requires a file path\n";
        return false;
      }
      options.log_path = argv[i + 1];
      ++i;
      continue;
    }

    std::cerr << "causis: unknown run option '" << flag << "'\n";
    return false;
  }

  return true;
}

void print_run_usage() {
  std::cerr << "causis: usage: causis run <program.ls> [--ticks N] [--log tick-log.json]\n";
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
    if (argc < 3) {
      print_run_usage();
      return 1;
    }

    RunOptions options;
    if (!parse_run_command(argc, argv, options)) {
      print_run_usage();
      return 1;
    }

    return run_simulation(options);
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
