#include "lexer/lexer.h"
#include "parser/parser.h"
#include "ir/lower.h"
#include "ir/optimizer.h"
#include "semantic/analyzer.h"

#include <optional>
#include <sstream>
#include <string>

#include <gtest/gtest.h>

namespace causis {
namespace {

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

ir::IrProgram lower(const ast::Program& program) {
  const ir::LowerResult result = ir::lower_program(program);
  EXPECT_TRUE(result.ok);
  EXPECT_TRUE(result.program.has_value());
  return *result.program;
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

bool has_push_int(const ir::IrProgram& program, int value) {
  for (const ir::Instruction& instruction : program.instructions) {
    if (instruction.opcode == ir::Opcode::PushInt && instruction.int_value == value) {
      return true;
    }
  }
  return false;
}

bool has_push_bool(const ir::IrProgram& program, bool value) {
  for (const ir::Instruction& instruction : program.instructions) {
    if (instruction.opcode == ir::Opcode::PushBool && instruction.bool_value == value) {
      return true;
    }
  }
  return false;
}

} // namespace
} // namespace causis

TEST(OptimizerConstantFold, FoldsIntegerAddition) {
  const std::unique_ptr<causis::ast::Program> program = causis::compile_program(R"(
world 5 5;
robot R at 0 0;
behavior R {
    every tick {
        speed = 2 + 3;
        move_right();
    }
}
)");

  ASSERT_NE(program, nullptr);

  const causis::ir::IrProgram raw = causis::lower(*program);
  const causis::ir::IrProgram optimized = causis::ir::optimize_program(raw);

  EXPECT_TRUE(causis::has_push_int(optimized, 5));
  EXPECT_EQ(causis::count_opcode(optimized, causis::ir::Opcode::Add), 0);
}

TEST(OptimizerConstantFold, FoldsChainedArithmetic) {
  const std::unique_ptr<causis::ast::Program> program = causis::compile_program(R"(
world 5 5;
robot R at 0 0;
behavior R {
    every tick {
        speed = 1 + 2 + 3;
    }
}
)");

  ASSERT_NE(program, nullptr);

  const causis::ir::IrProgram raw = causis::lower(*program);
  const causis::ir::IrProgram optimized = causis::ir::optimize_program(raw);

  EXPECT_TRUE(causis::has_push_int(optimized, 6));
  EXPECT_EQ(causis::count_opcode(optimized, causis::ir::Opcode::Add), 0);
}

TEST(OptimizerConstantFold, DoesNotFoldDivisionByZero) {
  const std::unique_ptr<causis::ast::Program> program = causis::compile_program(R"(
world 5 5;
robot R at 0 0;
behavior R {
    every tick {
        x = 1 / 0;
    }
}
)");

  ASSERT_NE(program, nullptr);

  const causis::ir::IrProgram raw = causis::lower(*program);
  const causis::ir::IrProgram optimized = causis::ir::optimize_program(raw);

  EXPECT_EQ(causis::count_opcode(raw, causis::ir::Opcode::Div), 1);
  EXPECT_EQ(causis::count_opcode(optimized, causis::ir::Opcode::Div), 1);
  EXPECT_TRUE(causis::has_push_int(optimized, 1));
  EXPECT_TRUE(causis::has_push_int(optimized, 0));
}

TEST(OptimizerConstantFold, FoldsUnaryNeg) {
  const std::unique_ptr<causis::ast::Program> program = causis::compile_program(R"(
world 5 5;
robot R at 0 0;
behavior R {
    every tick {
        speed = -5;
    }
}
)");

  ASSERT_NE(program, nullptr);

  const causis::ir::IrProgram raw = causis::lower(*program);
  const causis::ir::IrProgram optimized = causis::ir::optimize_program(raw);

  EXPECT_TRUE(causis::has_push_int(optimized, -5));
  EXPECT_EQ(causis::count_opcode(optimized, causis::ir::Opcode::Neg), 0);
}

TEST(OptimizerConstantFold, FoldsUnaryNot) {
  const std::unique_ptr<causis::ast::Program> program = causis::compile_program(R"(
world 5 5;
robot R at 0 0;
behavior R {
    every tick {
        flag = !false;
    }
}
)");

  ASSERT_NE(program, nullptr);

  const causis::ir::IrProgram raw = causis::lower(*program);
  const causis::ir::IrProgram optimized = causis::ir::optimize_program(raw);

  EXPECT_TRUE(causis::has_push_bool(optimized, true));
  EXPECT_EQ(causis::count_opcode(optimized, causis::ir::Opcode::Not), 0);
}

TEST(OptimizerDeadCode, RemovesFalseBranch) {
  const std::unique_ptr<causis::ast::Program> program = causis::compile_program(R"(
world 5 5;
robot R at 0 0;
behavior R {
    every tick {
        if false {
            move_right();
        }
        move_left();
    }
}
)");

  ASSERT_NE(program, nullptr);

  const causis::ir::IrProgram raw = causis::lower(*program);
  const causis::ir::IrProgram optimized = causis::ir::optimize_program(raw);

  EXPECT_EQ(causis::count_opcode(raw, causis::ir::Opcode::MoveRight), 1);
  EXPECT_EQ(causis::count_opcode(optimized, causis::ir::Opcode::MoveRight), 0);
  EXPECT_EQ(causis::count_opcode(optimized, causis::ir::Opcode::MoveLeft), 1);
}

TEST(OptimizerDeadCode, KeepsTrueBranchInIfElse) {
  const std::unique_ptr<causis::ast::Program> program = causis::compile_program(R"(
world 5 5;
robot R at 0 0;
behavior R {
    every tick {
        if true {
            move_right();
        } else {
            move_left();
        }
    }
}
)");

  ASSERT_NE(program, nullptr);

  const causis::ir::IrProgram raw = causis::lower(*program);
  const causis::ir::IrProgram optimized = causis::ir::optimize_program(raw);

  EXPECT_EQ(causis::count_opcode(optimized, causis::ir::Opcode::MoveRight), 1);
  EXPECT_EQ(causis::count_opcode(optimized, causis::ir::Opcode::MoveLeft), 0);
}

TEST(OptimizerDeadCode, FoldsComparisonAndRemovesFalseBranch) {
  const std::unique_ptr<causis::ast::Program> program = causis::compile_program(R"(
world 5 5;
robot R at 0 0;
behavior R {
    every tick {
        if 2 < 1 {
            move_right();
        }
        move_left();
    }
}
)");

  ASSERT_NE(program, nullptr);

  const causis::ir::IrProgram raw = causis::lower(*program);
  const causis::ir::IrProgram optimized = causis::ir::optimize_program(raw);

  EXPECT_EQ(causis::count_opcode(optimized, causis::ir::Opcode::MoveRight), 0);
  EXPECT_EQ(causis::count_opcode(optimized, causis::ir::Opcode::MoveLeft), 1);
}

TEST(OptimizerStopUnreachable, RemovesCodeAfterStraightLineStop) {
  const std::unique_ptr<causis::ast::Program> program = causis::compile_program(R"(
world 5 5;
robot R at 0 0;
behavior R {
    every tick {
        stop();
        move_right();
        move_left();
    }
}
)");

  ASSERT_NE(program, nullptr);

  const causis::ir::IrProgram raw = causis::lower(*program);
  const causis::ir::OptimizationResult result = causis::ir::optimize_program_with_stats(raw);

  EXPECT_EQ(causis::count_opcode(raw, causis::ir::Opcode::MoveRight), 1);
  EXPECT_EQ(causis::count_opcode(raw, causis::ir::Opcode::MoveLeft), 1);
  EXPECT_EQ(causis::count_opcode(result.program, causis::ir::Opcode::MoveRight), 0);
  EXPECT_EQ(causis::count_opcode(result.program, causis::ir::Opcode::MoveLeft), 0);
  EXPECT_LT(result.stats.ir_instructions_after, result.stats.ir_instructions_before);
}

TEST(OptimizerRedundantTurns, RemovesOppositeTurnPair) {
  const std::unique_ptr<causis::ast::Program> program = causis::compile_program(R"(
world 5 5;
robot R at 0 0;
behavior R {
    every tick {
        turn_left();
        turn_right();
        move_forward();
    }
}
)");

  ASSERT_NE(program, nullptr);

  const causis::ir::IrProgram raw = causis::lower(*program);
  const causis::ir::OptimizationResult result = causis::ir::optimize_program_with_stats(raw);

  EXPECT_EQ(causis::count_opcode(raw, causis::ir::Opcode::TurnLeft), 1);
  EXPECT_EQ(causis::count_opcode(raw, causis::ir::Opcode::TurnRight), 1);
  EXPECT_EQ(causis::count_opcode(result.program, causis::ir::Opcode::TurnLeft), 0);
  EXPECT_EQ(causis::count_opcode(result.program, causis::ir::Opcode::TurnRight), 0);
  EXPECT_EQ(causis::count_opcode(result.program, causis::ir::Opcode::MoveForward), 1);
}

TEST(OptimizerStage13, PathToTargetExampleReducesInstructionCount) {
  const std::unique_ptr<causis::ast::Program> program = causis::compile_program(R"(
world 20 20;
robot R at 2 2;
target T at 17 17;
obstacle at 8 5;
obstacle at 8 6;
obstacle at 8 7;
obstacle at 8 8;
behavior R {
    every tick {
        if distance_to(T) == 0 {
            stop();
        }
        if obstacle_ahead() {
            turn_right();
            move_forward();
        } else {
            move_toward(T);
            if collision() {
                turn_right();
            }
        }
    }
}
)");

  ASSERT_NE(program, nullptr);

  const causis::ir::IrProgram raw = causis::lower(*program);
  const causis::ir::OptimizationResult result = causis::ir::optimize_program_with_stats(raw);

  EXPECT_LT(result.stats.ir_instructions_after, result.stats.ir_instructions_before);
}
