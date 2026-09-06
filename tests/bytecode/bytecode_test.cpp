#include "lexer/lexer.h"
#include "parser/parser.h"
#include "bytecode/compiler.h"
#include "bytecode/disassembler.h"
#include "ir/lower.h"
#include "ir/optimizer.h"
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

void compile_source(const std::string& source, bytecode::Program& out) {
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

std::optional<std::size_t> instruction_index(const bytecode::Program& program,
                                             bytecode::Opcode opcode) {
  for (std::size_t i = 0; i < program.code.size(); ++i) {
    if (program.code[i].opcode == opcode) {
      return i;
    }
  }
  return std::nullopt;
}

bool disassembly_contains(const bytecode::Program& program, const std::string& text) {
  const std::string rendered = bytecode::disassemble_program(program);
  return rendered.find(text) != std::string::npos;
}

int count_opcode(const bytecode::Program& program, bytecode::Opcode opcode) {
  int count = 0;
  for (const bytecode::Instruction& instruction : program.code) {
    if (instruction.opcode == opcode) {
      ++count;
    }
  }
  return count;
}

int count_push_const_pool_index(const bytecode::Program& program, int32_t pool_index) {
  int count = 0;
  for (const bytecode::Instruction& instruction : program.code) {
    if (instruction.opcode == bytecode::Opcode::PushConst && instruction.a == pool_index) {
      ++count;
    }
  }
  return count;
}

} // namespace
} // namespace causis

TEST(BytecodeCompile, CompilesBasicMoveExample) {
  const std::string source = causis::read_file("examples/basic_move.ls");
  ASSERT_FALSE(source.empty());

  causis::bytecode::Program program;
  causis::compile_source(source, program);

  EXPECT_TRUE(causis::disassembly_contains(program, "CREATE_WORLD"));
  EXPECT_TRUE(causis::disassembly_contains(program, "CREATE_ROBOT"));
  EXPECT_TRUE(causis::disassembly_contains(program, "BEGIN_TICK"));
  EXPECT_TRUE(causis::disassembly_contains(program, "LOAD_AGENT"));
  EXPECT_TRUE(causis::disassembly_contains(program, "MOVE_RIGHT"));
  EXPECT_TRUE(causis::disassembly_contains(program, "END_TICK"));
  EXPECT_TRUE(causis::disassembly_contains(program, "JUMP"));
  EXPECT_TRUE(causis::disassembly_contains(program, "HALT"));
}

TEST(BytecodeCompile, ResolvesJumpTargets) {
  causis::bytecode::Program program;
  causis::compile_source(R"(
world 5 5;
robot R at 0 0;
behavior R {
    every tick {
        move_right();
    }
}
)", program);

  const std::optional<std::size_t> begin_tick =
      causis::instruction_index(program, causis::bytecode::Opcode::BeginTick);
  ASSERT_TRUE(begin_tick.has_value());

  ASSERT_GT(causis::count_opcode(program, causis::bytecode::Opcode::Jump), 0);

  for (const causis::bytecode::Instruction& instruction : program.code) {
    if (instruction.opcode == causis::bytecode::Opcode::Jump) {
      EXPECT_EQ(static_cast<std::size_t>(instruction.a), *begin_tick);
    }
  }
}

TEST(BytecodeCompile, UsesConstantPoolForLiterals) {
  causis::bytecode::Program program;
  causis::compile_source(R"(
world 5 5;
robot R at 0 0;
behavior R {
    every tick {
        speed = 2 + 3;
    }
}
)", program);

  EXPECT_FALSE(program.int_constants.empty());
  EXPECT_TRUE(causis::disassembly_contains(program, "PUSH_CONST"));
  EXPECT_TRUE(causis::disassembly_contains(program, "STORE_VAR"));
}

TEST(BytecodeCompile, DeduplicatesRepeatedIntConstants) {
  const std::unique_ptr<causis::ast::Program> program = causis::compile_program(R"(
world 5 5;
robot R at 0 0;
behavior R {
    every tick {
        speed = 2 + 2;
    }
}
)");
  ASSERT_NE(program, nullptr);

  const causis::ir::LowerResult lower_result = causis::ir::lower_program(*program);
  ASSERT_TRUE(lower_result.ok);
  ASSERT_TRUE(lower_result.program.has_value());

  const causis::bytecode::CompileResult compile_result =
      causis::bytecode::compile_ir(*lower_result.program);
  ASSERT_TRUE(compile_result.ok);
  ASSERT_TRUE(compile_result.program.has_value());

  const causis::bytecode::Program& bytecode = *compile_result.program;
  EXPECT_EQ(bytecode.int_constants.size(), 1U);
  EXPECT_EQ(bytecode.int_constants[0], 2);
  EXPECT_EQ(causis::count_push_const_pool_index(bytecode, 0), 2);
}

TEST(BytecodeCompile, ReportsUnknownLabel) {
  causis::ir::IrProgram ir;
  causis::ir::Instruction jump;
  jump.opcode = causis::ir::Opcode::Jump;
  jump.text = "missing_label";
  jump.line = 3;
  jump.column = 5;
  ir.instructions.push_back(jump);

  const causis::bytecode::CompileResult compile_result = causis::bytecode::compile_ir(ir);
  ASSERT_FALSE(compile_result.ok);
  ASSERT_TRUE(compile_result.error.has_value());
  EXPECT_NE(compile_result.error->message.find("unknown label"), std::string::npos);
  EXPECT_NE(compile_result.error->message.find("missing_label"), std::string::npos);
  EXPECT_EQ(compile_result.error->line, 3);
  EXPECT_EQ(compile_result.error->column, 5);
}

TEST(BytecodeCompile, CompilesCollisionExample) {
  const std::string source = causis::read_file("examples/collision.ls");
  ASSERT_FALSE(source.empty());

  causis::bytecode::Program program;
  causis::compile_source(source, program);

  EXPECT_TRUE(causis::disassembly_contains(program, "OBSTACLE_AHEAD"));
  EXPECT_TRUE(causis::disassembly_contains(program, "JUMP_IF_FALSE"));
  EXPECT_TRUE(causis::disassembly_contains(program, "TURN_RIGHT"));
  EXPECT_TRUE(causis::disassembly_contains(program, "MOVE_FORWARD"));
}

TEST(BytecodeCompile, CompilesTargetExample) {
  const std::string source = causis::read_file("examples/target.ls");
  ASSERT_FALSE(source.empty());

  causis::bytecode::Program program;
  causis::compile_source(source, program);

  EXPECT_TRUE(causis::disassembly_contains(program, "CREATE_TARGET"));
  EXPECT_TRUE(causis::disassembly_contains(program, "MOVE_TOWARD"));
}

TEST(BytecodeDisassemble, PrintsInstructionAddresses) {
  causis::bytecode::Program program;
  causis::bytecode::Instruction instruction;
  instruction.opcode = causis::bytecode::Opcode::MoveRight;
  program.code.push_back(instruction);

  const std::string rendered = causis::bytecode::disassemble_program(program);
  EXPECT_NE(rendered.find("0000 MOVE_RIGHT"), std::string::npos);
}
