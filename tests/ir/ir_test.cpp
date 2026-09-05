#include "lexer/lexer.h"
#include "parser/parser.h"
#include "ir/lower.h"
#include "semantic/analyzer.h"

#include <fstream>
#include <optional>
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

std::unique_ptr<ast::Program> parse_program_only(const std::string& source) {
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

  return std::move(parse_result.program);
}

bool ir_contains(const ir::IrProgram& program, const std::string& text) {
  const std::string rendered = ir::print_ir(program);
  return rendered.find(text) != std::string::npos;
}

int count_opcode(const ir::IrProgram& program, ir::Opcode opcode) {
  int count = 0;
  for (const ir::Instruction& instruction : program.instructions) {
    if (instruction.opcode == opcode) {
      ++count;
    }
  }
  return count;
}

std::optional<std::size_t> instruction_index(const ir::IrProgram& program, ir::Opcode opcode,
                                               const std::string& text = "") {
  for (std::size_t i = 0; i < program.instructions.size(); ++i) {
    const ir::Instruction& instruction = program.instructions[i];
    if (instruction.opcode != opcode) {
      continue;
    }
    if (!text.empty() && instruction.text != text) {
      continue;
    }
    return i;
  }
  return std::nullopt;
}

} // namespace
} // namespace causis

TEST(IrLower, LowersWorldAndRobotSetup) {
  const std::string source = causis::read_file("examples/basic_move.ls");
  ASSERT_FALSE(source.empty());

  const std::unique_ptr<causis::ast::Program> program = causis::compile_program(source);
  ASSERT_NE(program, nullptr);

  const causis::ir::LowerResult result = causis::ir::lower_program(*program);
  ASSERT_TRUE(result.ok);
  ASSERT_TRUE(result.program.has_value());
  EXPECT_TRUE(causis::ir_contains(*result.program, "WORLD 8 5"));
  EXPECT_TRUE(causis::ir_contains(*result.program, "ROBOT R 0 2"));
}

TEST(IrLower, LowersTickLoopAndMoveRight) {
  const std::string source = causis::read_file("examples/basic_move.ls");
  const std::unique_ptr<causis::ast::Program> program = causis::compile_program(source);
  ASSERT_NE(program, nullptr);

  const causis::ir::LowerResult result = causis::ir::lower_program(*program);
  ASSERT_TRUE(result.ok);

  EXPECT_TRUE(causis::ir_contains(*result.program, "LABEL tick_loop"));
  EXPECT_TRUE(causis::ir_contains(*result.program, "BEGIN_TICK"));
  EXPECT_TRUE(causis::ir_contains(*result.program, "LOAD_AGENT R"));
  EXPECT_TRUE(causis::ir_contains(*result.program, "MOVE_RIGHT"));
  EXPECT_TRUE(causis::ir_contains(*result.program, "END_TICK"));
  EXPECT_TRUE(causis::ir_contains(*result.program, "JUMP tick_loop"));
}

TEST(IrLower, OrdersGlobalTickFrameAroundAllRobots) {
  const std::unique_ptr<causis::ast::Program> program = causis::compile_program(R"(
world 5 5;
robot R at 0 0;
robot S at 1 0;
behavior R {
    every tick {
        move_right();
    }
}
behavior S {
    every tick {
        move_left();
    }
}
)");

  ASSERT_NE(program, nullptr);

  const causis::ir::LowerResult result = causis::ir::lower_program(*program);
  ASSERT_TRUE(result.ok);
  ASSERT_TRUE(result.program.has_value());

  const causis::ir::IrProgram& ir = *result.program;

  EXPECT_EQ(causis::count_opcode(ir, causis::ir::Opcode::BeginTick), 1);
  EXPECT_EQ(causis::count_opcode(ir, causis::ir::Opcode::EndTick), 1);

  const std::optional<std::size_t> tick_loop =
      causis::instruction_index(ir, causis::ir::Opcode::Label, "tick_loop");
  const std::optional<std::size_t> begin_tick =
      causis::instruction_index(ir, causis::ir::Opcode::BeginTick);
  const std::optional<std::size_t> load_r =
      causis::instruction_index(ir, causis::ir::Opcode::LoadAgent, "R");
  const std::optional<std::size_t> load_s =
      causis::instruction_index(ir, causis::ir::Opcode::LoadAgent, "S");
  const std::optional<std::size_t> end_tick =
      causis::instruction_index(ir, causis::ir::Opcode::EndTick);
  const std::optional<std::size_t> jump_tick_loop =
      causis::instruction_index(ir, causis::ir::Opcode::Jump, "tick_loop");

  ASSERT_TRUE(tick_loop.has_value());
  ASSERT_TRUE(begin_tick.has_value());
  ASSERT_TRUE(load_r.has_value());
  ASSERT_TRUE(load_s.has_value());
  ASSERT_TRUE(end_tick.has_value());
  ASSERT_TRUE(jump_tick_loop.has_value());

  EXPECT_LT(*tick_loop, *begin_tick);
  EXPECT_LT(*begin_tick, *load_r);
  EXPECT_LT(*load_r, *load_s);
  EXPECT_LT(*load_s, *end_tick);
  EXPECT_LT(*end_tick, *jump_tick_loop);
}

TEST(IrLower, LowersIfElseBranches) {
  const std::string source = causis::read_file("examples/collision.ls");
  const std::unique_ptr<causis::ast::Program> program = causis::compile_program(source);
  ASSERT_NE(program, nullptr);

  const causis::ir::LowerResult result = causis::ir::lower_program(*program);
  ASSERT_TRUE(result.ok);

  EXPECT_TRUE(causis::ir_contains(*result.program, "OBSTACLE_AHEAD"));
  EXPECT_TRUE(causis::ir_contains(*result.program, "JUMP_IF_FALSE"));
  EXPECT_TRUE(causis::ir_contains(*result.program, "TURN_RIGHT"));
  EXPECT_TRUE(causis::ir_contains(*result.program, "MOVE_FORWARD"));
}

TEST(IrLower, LowersMoveTowardTarget) {
  const std::string source = causis::read_file("examples/target.ls");
  const std::unique_ptr<causis::ast::Program> program = causis::compile_program(source);
  ASSERT_NE(program, nullptr);

  const causis::ir::LowerResult result = causis::ir::lower_program(*program);
  ASSERT_TRUE(result.ok);

  EXPECT_TRUE(causis::ir_contains(*result.program, "TARGET T 8 8"));
  EXPECT_TRUE(causis::ir_contains(*result.program, "MOVE_TOWARD T"));
}

TEST(IrPrint, FormatsInstructionsReadably) {
  causis::ir::IrProgram program;
  causis::ir::Instruction instruction;
  instruction.opcode = causis::ir::Opcode::MoveRight;
  program.instructions.push_back(instruction);

  EXPECT_EQ(causis::ir::format_instruction(instruction), "MOVE_RIGHT");
}

TEST(IrLower, ReportsUnknownFunction) {
  const std::unique_ptr<causis::ast::Program> program = causis::parse_program_only(R"(
world 5 5;
robot R at 0 0;
behavior R {
    every tick {
        jump();
    }
}
)");

  ASSERT_NE(program, nullptr);

  const causis::ir::LowerResult result = causis::ir::lower_program(*program);
  ASSERT_FALSE(result.ok);
  ASSERT_TRUE(result.error.has_value());
  EXPECT_NE(result.error->message.find("unknown function"), std::string::npos);
}

TEST(IrLower, KeepsStatementsAfterConditionalStop) {
  const std::unique_ptr<causis::ast::Program> program = causis::compile_program(R"(
world 5 5;
robot R at 0 0;
obstacle at 1 0;
behavior R {
    every tick {
        if obstacle_ahead() {
            stop();
        }
        move_right();
    }
}
)");

  ASSERT_NE(program, nullptr);

  const causis::ir::LowerResult result = causis::ir::lower_program(*program);
  ASSERT_TRUE(result.ok);
  ASSERT_TRUE(result.program.has_value());

  const causis::ir::IrProgram& ir = *result.program;

  const std::optional<std::size_t> stop =
      causis::instruction_index(ir, causis::ir::Opcode::Stop);
  const std::optional<std::size_t> move_right =
      causis::instruction_index(ir, causis::ir::Opcode::MoveRight);

  ASSERT_TRUE(stop.has_value());
  ASSERT_TRUE(move_right.has_value());
  ASSERT_LT(*stop + 1, ir.instructions.size());

  const causis::ir::Instruction& jump_after_stop = ir.instructions[*stop + 1];
  EXPECT_EQ(jump_after_stop.opcode, causis::ir::Opcode::Jump);
  EXPECT_EQ(jump_after_stop.text.find("robot_end"), 0U);

  EXPECT_LT(*stop + 1, *move_right);

  const std::optional<std::size_t> robot_end_label =
      causis::instruction_index(ir, causis::ir::Opcode::Label, jump_after_stop.text);
  ASSERT_TRUE(robot_end_label.has_value());
  EXPECT_LT(*move_right, *robot_end_label);
}
