// Stage 14 — simple performance harness (compile stages, VM throughput, end-to-end).
#include "bytecode/compiler.h"
#include "cli/frontend.h"
#include "ir/lower.h"
#include "ir/optimizer.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "semantic/analyzer.h"
#include "vm/vm.h"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace {

using Clock = std::chrono::steady_clock;

constexpr int kDefaultIterations = 20;
constexpr int kDefaultVmTicks = 10'000;
constexpr int kDefaultE2eTicks = 1'000;

struct StageTimings {
  double tokenize_ms{0};
  double parse_ms{0};
  double semantic_ms{0};
  double lower_ms{0};
  double optimize_ms{0};
  double bytecode_ms{0};
};

struct CompileArtifacts {
  bool ok{false};
  std::string error;
  std::unique_ptr<causis::ast::Program> program;
  causis::ir::IrProgram raw_ir;
  causis::ir::IrProgram optimized_ir;
  causis::bytecode::Program raw_bytecode;
  causis::bytecode::Program optimized_bytecode;
};

double to_ms(Clock::duration duration) {
  return std::chrono::duration<double, std::milli>(duration).count();
}

double median(std::vector<double> values) {
  if (values.empty()) {
    return 0.0;
  }

  std::sort(values.begin(), values.end());
  const std::size_t mid = values.size() / 2;
  if (values.size() % 2 == 0) {
    return (values[mid - 1] + values[mid]) / 2.0;
  }

  return values[mid];
}

bool read_file(const std::string& path, std::string& source) {
  std::ifstream file(path);
  if (!file) {
    return false;
  }

  std::ostringstream buffer;
  buffer << file.rdbuf();
  source = buffer.str();
  return true;
}

bool measure_one_compile_pass(const std::string& source, StageTimings& timings, CompileArtifacts& artifacts) {
  const auto t0 = Clock::now();
  causis::lexer::Lexer lexer(source);
  const causis::lexer::TokenizeResult lex_result = lexer.tokenize();
  const auto t1 = Clock::now();
  if (lex_result.error.has_value()) {
    artifacts.error = lex_result.error->message;
    return false;
  }

  causis::parser::Parser parser(lex_result.tokens);
  causis::parser::ParseResult parse_result = parser.parse_program();
  const auto t2 = Clock::now();
  if (parse_result.error.has_value()) {
    artifacts.error = parse_result.error->message;
    return false;
  }

  causis::semantic::Analyzer analyzer;
  const causis::semantic::SemanticResult semantic_result = analyzer.analyze(*parse_result.program);
  const auto t3 = Clock::now();
  if (semantic_result.error.has_value()) {
    artifacts.error = semantic_result.error->message;
    return false;
  }

  const causis::ir::LowerResult lower_result = causis::ir::lower_program(*parse_result.program);
  const auto t4 = Clock::now();
  if (lower_result.error.has_value() || !lower_result.program.has_value()) {
    artifacts.error = lower_result.error.has_value() ? lower_result.error->message
                                                     : "IR lowering produced no program";
    return false;
  }

  const causis::ir::IrProgram optimized = causis::ir::optimize_program(*lower_result.program);
  const auto t5 = Clock::now();

  const causis::bytecode::CompileResult raw_compile =
      causis::bytecode::compile_ir(*lower_result.program);
  const causis::bytecode::CompileResult opt_compile = causis::bytecode::compile_ir(optimized);
  const auto t6 = Clock::now();

  if (!raw_compile.ok || !raw_compile.program.has_value() || !opt_compile.ok ||
      !opt_compile.program.has_value()) {
    artifacts.error = "bytecode compilation failed";
    return false;
  }

  timings.tokenize_ms = to_ms(t1 - t0);
  timings.parse_ms = to_ms(t2 - t1);
  timings.semantic_ms = to_ms(t3 - t2);
  timings.lower_ms = to_ms(t4 - t3);
  timings.optimize_ms = to_ms(t5 - t4);
  timings.bytecode_ms = to_ms(t6 - t5);

  artifacts.ok = true;
  artifacts.program = std::move(parse_result.program);
  artifacts.raw_ir = *lower_result.program;
  artifacts.optimized_ir = std::move(optimized);
  artifacts.raw_bytecode = *raw_compile.program;
  artifacts.optimized_bytecode = *opt_compile.program;
  return true;
}

bool build_artifacts_from_file(const std::string& path, CompileArtifacts& artifacts) {
  std::string source;
  if (!read_file(path, source)) {
    artifacts.error = "could not read file '" + path + "'";
    return false;
  }

  StageTimings timings;
  return measure_one_compile_pass(source, timings, artifacts);
}

void print_usage() {
  std::cout << "causis_bench — Stage 14 performance harness\n\n"
            << "Usage:\n"
            << "  causis_bench compile <program.ls> [--iterations N]\n"
            << "  causis_bench vm <program.ls> [--ticks N] [--compare-opt]\n"
            << "  causis_bench e2e <program.ls> [--ticks N] [--compare-opt]\n"
            << "  causis_bench suite [--iterations N]\n";
}

bool parse_positive_int(const char* text, int& value) {
  try {
    std::size_t consumed = 0;
    const long long parsed = std::stoll(text, &consumed);
    if (consumed == 0 || text[consumed] != '\0' || parsed <= 0) {
      return false;
    }
    value = static_cast<int>(parsed);
    return true;
  } catch (const std::exception&) {
    return false;
  }
}

int run_compile_benchmark(const std::string& path, int iterations) {
  std::string source;
  if (!read_file(path, source)) {
    std::cerr << "causis_bench: could not read '" << path << "'\n";
    return 1;
  }

  StageTimings warmup;
  CompileArtifacts warmup_artifacts;
  if (!measure_one_compile_pass(source, warmup, warmup_artifacts)) {
    std::cerr << "causis_bench: compile failed: " << warmup_artifacts.error << '\n';
    return 1;
  }

  std::vector<double> tokenize;
  std::vector<double> parse;
  std::vector<double> semantic;
  std::vector<double> lower;
  std::vector<double> optimize;
  std::vector<double> bytecode;

  tokenize.reserve(iterations);
  parse.reserve(iterations);
  semantic.reserve(iterations);
  lower.reserve(iterations);
  optimize.reserve(iterations);
  bytecode.reserve(iterations);

  for (int i = 0; i < iterations; ++i) {
    StageTimings sample;
    CompileArtifacts artifacts;
    if (!measure_one_compile_pass(source, sample, artifacts)) {
      std::cerr << "causis_bench: compile failed on iteration " << i << '\n';
      return 1;
    }

    tokenize.push_back(sample.tokenize_ms);
    parse.push_back(sample.parse_ms);
    semantic.push_back(sample.semantic_ms);
    lower.push_back(sample.lower_ms);
    optimize.push_back(sample.optimize_ms);
    bytecode.push_back(sample.bytecode_ms);
  }

  std::cout << std::fixed << std::setprecision(3);
  std::cout << "Compile stage timings (median of " << iterations << " runs)\n";
  std::cout << "Program: " << path << "\n";
  std::cout << "-----------------------\n";
  std::cout << "  tokenize : " << median(tokenize) << " ms\n";
  std::cout << "  parse    : " << median(parse) << " ms\n";
  std::cout << "  semantic : " << median(semantic) << " ms\n";
  std::cout << "  lower    : " << median(lower) << " ms\n";
  std::cout << "  optimize : " << median(optimize) << " ms\n";
  std::cout << "  bytecode : " << median(bytecode) << " ms\n";

  const double total = median(tokenize) + median(parse) + median(semantic) + median(lower) +
                       median(optimize) + median(bytecode);
  std::cout << "  total    : " << total << " ms\n";
  return 0;
}

double measure_vm_ticks(const causis::bytecode::Program& program, int tick_count) {
  const auto start = Clock::now();
  const causis::vm::VmResult result = causis::vm::run_bytecode(program, tick_count);
  const auto end = Clock::now();
  if (!result.ok) {
    return -1.0;
  }

  return to_ms(end - start);
}

void print_vm_line(const std::string& label, int tick_count, double elapsed_ms) {
  std::cout << "  " << label << " (" << tick_count << " ticks): ";
  if (elapsed_ms < 0.0) {
    std::cout << "VM failed\n";
    return;
  }

  const double ticks_per_sec = (elapsed_ms > 0.0) ? (tick_count / (elapsed_ms / 1000.0)) : 0.0;
  std::cout << elapsed_ms << " ms, " << std::setprecision(1) << ticks_per_sec << " ticks/sec\n";
  std::cout << std::setprecision(3);
}

int run_vm_benchmark(const std::string& path, int tick_count, bool compare_opt) {
  CompileArtifacts artifacts;
  if (!build_artifacts_from_file(path, artifacts)) {
    std::cerr << "causis_bench: compile failed: " << artifacts.error << '\n';
    return 1;
  }

  std::cout << std::fixed << std::setprecision(3);
  std::cout << "VM throughput (compile once, time VM only)\n";
  std::cout << "Program: " << path << "\n";
  std::cout << "-----------------------\n";

  if (compare_opt) {
    const std::vector<int> sweep = {1'000, 10'000, 100'000};
    for (int ticks : sweep) {
      print_vm_line("optimized", ticks, measure_vm_ticks(artifacts.optimized_bytecode, ticks));
      print_vm_line("raw IR", ticks, measure_vm_ticks(artifacts.raw_bytecode, ticks));
    }
    return 0;
  }

  const double elapsed = measure_vm_ticks(artifacts.optimized_bytecode, tick_count);
  print_vm_line("optimized", tick_count, elapsed);
  return elapsed < 0.0 ? 1 : 0;
}

int run_e2e_benchmark(const std::string& path, int tick_count, bool compare_opt) {
  std::string source;
  if (!read_file(path, source)) {
    std::cerr << "causis_bench: could not read '" << path << "'\n";
    return 1;
  }

  const auto compile_start = Clock::now();
  CompileArtifacts artifacts;
  StageTimings compile_sample;
  if (!measure_one_compile_pass(source, compile_sample, artifacts)) {
    std::cerr << "causis_bench: compile failed: " << artifacts.error << '\n';
    return 1;
  }
  const auto compile_end = Clock::now();
  const double compile_ms = to_ms(compile_end - compile_start);

  std::cout << std::fixed << std::setprecision(3);
  std::cout << "End-to-end (compile once + VM execute)\n";
  std::cout << "Program: " << path << "\n";
  std::cout << "-----------------------\n";
  std::cout << "  compile (full pipeline) : " << compile_ms << " ms\n";

  if (compare_opt) {
    const double vm_raw = measure_vm_ticks(artifacts.raw_bytecode, tick_count);
    const double vm_opt = measure_vm_ticks(artifacts.optimized_bytecode, tick_count);
    print_vm_line("VM raw IR", tick_count, vm_raw);
    print_vm_line("VM optimized", tick_count, vm_opt);
    if (vm_raw >= 0.0 && vm_opt >= 0.0) {
      std::cout << "  total raw       : " << (compile_ms + vm_raw) << " ms\n";
      std::cout << "  total optimized : " << (compile_ms + vm_opt) << " ms\n";
    }
    return (vm_raw < 0.0 || vm_opt < 0.0) ? 1 : 0;
  }

  const double vm_ms = measure_vm_ticks(artifacts.optimized_bytecode, tick_count);
  print_vm_line("VM optimized", tick_count, vm_ms);
  if (vm_ms < 0.0) {
    return 1;
  }

  std::cout << "  total (compile + VM): " << (compile_ms + vm_ms) << " ms\n";
  return 0;
}

int run_suite(int iterations) {
  const std::vector<std::string> compile_examples = {"examples/basic_move.ls", "examples/target.ls",
                                                   "examples/path_to_target.ls"};

  for (const std::string& example : compile_examples) {
    std::cout << '\n';
    if (run_compile_benchmark(example, iterations) != 0) {
      return 1;
    }
  }

  std::cout << "\n\n";
  if (run_vm_benchmark("examples/path_to_target.ls", 0, true) != 0) {
    return 1;
  }

  std::cout << "\n\n";
  if (run_e2e_benchmark("examples/path_to_target.ls", kDefaultE2eTicks, true) != 0) {
    return 1;
  }

  return 0;
}

} // namespace

int main(int argc, char* argv[]) {
  if (argc < 2) {
    print_usage();
    return 1;
  }

  const std::string command{argv[1]};
  if (command == "--help" || command == "-h") {
    print_usage();
    return 0;
  }

  if (command == "suite") {
    int iterations = kDefaultIterations;
    for (int i = 2; i < argc; ++i) {
      const std::string flag{argv[i]};
      if (flag == "--iterations" && i + 1 < argc) {
        if (!parse_positive_int(argv[i + 1], iterations)) {
          std::cerr << "causis_bench: invalid --iterations value\n";
          return 1;
        }
        ++i;
      } else {
        std::cerr << "causis_bench: unknown suite option '" << flag << "'\n";
        return 1;
      }
    }
    return run_suite(iterations);
  }

  if (argc < 3) {
    print_usage();
    return 1;
  }

  const std::string path{argv[2]};
  int iterations = kDefaultIterations;
  int tick_count = kDefaultVmTicks;
  if (command == "e2e") {
    tick_count = kDefaultE2eTicks;
  }
  bool compare_opt = false;

  for (int i = 3; i < argc; ++i) {
    const std::string flag{argv[i]};
    if (flag == "--iterations" && i + 1 < argc) {
      if (!parse_positive_int(argv[i + 1], iterations)) {
        std::cerr << "causis_bench: invalid --iterations value\n";
        return 1;
      }
      ++i;
      continue;
    }
    if (flag == "--ticks" && i + 1 < argc) {
      if (!parse_positive_int(argv[i + 1], tick_count)) {
        std::cerr << "causis_bench: invalid --ticks value\n";
        return 1;
      }
      ++i;
      continue;
    }
    if (flag == "--compare-opt") {
      compare_opt = true;
      continue;
    }

    std::cerr << "causis_bench: unknown option '" << flag << "'\n";
    return 1;
  }

  if (command == "compile") {
    return run_compile_benchmark(path, iterations);
  }
  if (command == "vm") {
    return run_vm_benchmark(path, tick_count, compare_opt);
  }
  if (command == "e2e") {
    return run_e2e_benchmark(path, tick_count, compare_opt);
  }

  std::cerr << "causis_bench: unknown command '" << command << "'\n";
  print_usage();
  return 1;
}
