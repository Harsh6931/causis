#include "lexer/lexer.h"
#include "parser/parser.h"
#include "bytecode/compiler.h"
#include "ir/lower.h"
#include "ir/optimizer.h"
#include "runtime/program_runner.h"
#include "semantic/analyzer.h"
#include "vm/vm.h"

#include <fstream>
#include <sstream>
#include <string>

#include <gtest/gtest.h>

namespace causis {
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

std::unique_ptr<ast::Program> compile_program(const std::string& source) {
  lexer::Lexer lexer(source);
  const lexer::TokenizeResult tokens = lexer.tokenize();
  if (tokens.error.has_value()) {
    return nullptr;
  }

  parser::Parser parser(tokens.tokens);
  parser::ParseResult parse_result = parser.parse_program();
  if (parse_result.error.has_value()) {
    return nullptr;
  }

  semantic::Analyzer analyzer;
  const semantic::SemanticResult semantic_result = analyzer.analyze(*parse_result.program);
  if (semantic_result.error.has_value()) {
    return nullptr;
  }

  return std::move(parse_result.program);
}

void compile_to_bytecode(const std::string& source, bytecode::Program& out) {
  const std::unique_ptr<ast::Program> program = compile_program(source);
  ASSERT_NE(program, nullptr);

  const ir::LowerResult lower_result = ir::lower_program(*program);
  ASSERT_TRUE(lower_result.ok);
  ASSERT_TRUE(lower_result.program.has_value());

  const ir::IrProgram optimized = ir::optimize_program(*lower_result.program);
  const bytecode::CompileResult compile_result = bytecode::compile_ir(optimized);
  ASSERT_TRUE(compile_result.ok);
  ASSERT_TRUE(compile_result.program.has_value());
  out = *compile_result.program;
}

} // namespace
} // namespace causis

TEST(VmRun, MatchesBasicMoveExample) {
  causis::bytecode::Program bytecode;
  causis::compile_to_bytecode(causis::read_file("examples/basic_move.ls"), bytecode);

  const causis::vm::VmResult result = causis::vm::run_bytecode(bytecode, 7);
  ASSERT_TRUE(result.ok);
  ASSERT_TRUE(result.simulation.has_value());

  const causis::runtime::Robot& robot = result.simulation->world().robot("R");
  EXPECT_EQ(robot.x, 7);
  EXPECT_EQ(robot.y, 2);
}

TEST(VmRun, ReportsDivisionByZero) {
  causis::bytecode::Program bytecode;
  causis::compile_to_bytecode(R"(
world 5 5;
robot R at 0 0;
behavior R {
    every tick {
        x = 1 / 0;
    }
}
)", bytecode);

  const causis::vm::VmResult result = causis::vm::run_bytecode(bytecode, 1);
  ASSERT_FALSE(result.ok);
  ASSERT_TRUE(result.error.has_value());
  EXPECT_NE(result.error->message.find("division by zero"), std::string::npos);
}

TEST(VmRun, StopSkipsRemainingRobotBody) {
  causis::bytecode::Program bytecode;
  causis::compile_to_bytecode(R"(
world 5 5;
robot R at 0 0;
behavior R {
    every tick {
        move_right();
        stop();
        move_right();
    }
}
)", bytecode);

  const causis::vm::VmResult result = causis::vm::run_bytecode(bytecode, 1);
  ASSERT_TRUE(result.ok);

  const causis::runtime::Robot& robot = result.simulation->world().robot("R");
  EXPECT_EQ(robot.x, 1);
  EXPECT_EQ(robot.y, 0);
}

TEST(VmRun, RunsRequestedNumberOfTicks) {
  causis::bytecode::Program bytecode;
  causis::compile_to_bytecode(causis::read_file("examples/basic_move.ls"), bytecode);

  const causis::vm::VmResult result = causis::vm::run_bytecode(bytecode, 3);
  ASSERT_TRUE(result.ok);
  EXPECT_EQ(result.simulation->tick_count(), 3);
}

TEST(VmRun, RunsZeroTicks) {
  causis::bytecode::Program bytecode;
  causis::compile_to_bytecode(R"(
world 5 5;
robot R at 0 0;
behavior R {
    every tick {
        move_right();
    }
}
)", bytecode);

  const causis::vm::VmResult result = causis::vm::run_bytecode(bytecode, 0);
  ASSERT_TRUE(result.ok);
  EXPECT_EQ(result.simulation->tick_count(), 0);

  const causis::runtime::Robot& robot = result.simulation->world().robot("R");
  EXPECT_EQ(robot.x, 0);
  EXPECT_EQ(robot.y, 0);
}

TEST(VmRun, StopDoesNotPreventOtherRobots) {
  causis::bytecode::Program bytecode;
  causis::compile_to_bytecode(R"(
world 5 5;
robot A at 0 0;
robot B at 4 0;
behavior A {
    every tick {
        move_right();
        stop();
        move_right();
    }
}
behavior B {
    every tick {
        move_left();
    }
}
)", bytecode);

  const causis::vm::VmResult result = causis::vm::run_bytecode(bytecode, 1);
  ASSERT_TRUE(result.ok);

  const causis::runtime::Robot& robot_a = result.simulation->world().robot("A");
  const causis::runtime::Robot& robot_b = result.simulation->world().robot("B");
  EXPECT_EQ(robot_a.x, 1);
  EXPECT_EQ(robot_b.x, 3);
}

TEST(VmRun, StopAllowsSubsequentTicks) {
  causis::bytecode::Program bytecode;
  causis::compile_to_bytecode(R"(
world 5 5;
robot R at 0 0;
behavior R {
    every tick {
        move_right();
        stop();
        move_right();
    }
}
)", bytecode);

  const causis::vm::VmResult result = causis::vm::run_bytecode(bytecode, 3);
  ASSERT_TRUE(result.ok);
  EXPECT_EQ(result.simulation->tick_count(), 3);

  const causis::runtime::Robot& robot = result.simulation->world().robot("R");
  EXPECT_EQ(robot.x, 3);
  EXPECT_EQ(robot.y, 0);
}
