#include "runtime/program_runner.h"
#include "runtime/tick_log.h"

#include "lexer/lexer.h"
#include "parser/parser.h"
#include "semantic/analyzer.h"

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

} // namespace
} // namespace causis

TEST(TickLog, CapturesRobotAndTargetPositions) {
  const std::unique_ptr<causis::ast::Program> program = causis::compile_program(R"(
world 5 5;
robot R at 1 1;
target T at 3 3;
behavior R {
    every tick {
        move_right();
    }
}
)");
  ASSERT_NE(program, nullptr);

  causis::runtime::TickLog log;
  const causis::runtime::RunResult result = causis::runtime::run_program(*program, 2, &log);
  ASSERT_TRUE(result.ok);

  EXPECT_EQ(log.width, 5);
  EXPECT_EQ(log.height, 5);
  EXPECT_EQ(log.frames.size(), 3U);

  EXPECT_EQ(log.frames[0].tick, 0);
  EXPECT_EQ(log.frames[0].robots[0].x, 1);
  EXPECT_EQ(log.frames[0].robots[0].y, 1);
  EXPECT_EQ(log.frames[0].targets[0].x, 3);

  EXPECT_EQ(log.frames[2].tick, 2);
  EXPECT_EQ(log.frames[2].robots[0].x, 3);
}

TEST(TickLog, JsonContainsFramesAndObstacles) {
  const std::unique_ptr<causis::ast::Program> program = causis::compile_program(R"(
world 4 4;
robot R at 0 0;
obstacle at 1 0;
behavior R {
    every tick {
        move_right();
    }
}
)");
  ASSERT_NE(program, nullptr);

  causis::runtime::TickLog log;
  const causis::runtime::RunResult result = causis::runtime::run_program(*program, 1, &log);
  ASSERT_TRUE(result.ok);

  const std::string json = causis::runtime::tick_log_to_json(log);
  EXPECT_NE(json.find("\"width\": 4"), std::string::npos);
  EXPECT_NE(json.find("\"frames\""), std::string::npos);
  EXPECT_NE(json.find("\"obstacles\""), std::string::npos);
  EXPECT_NE(json.find("\"x\": 1"), std::string::npos);
}

TEST(TickLog, ZeroTicksRecordsInitialFrameOnly) {
  const std::unique_ptr<causis::ast::Program> program = causis::compile_program(R"(
world 3 3;
robot R at 0 0;
behavior R {
    every tick {
        move_right();
    }
}
)");
  ASSERT_NE(program, nullptr);

  causis::runtime::TickLog log;
  const causis::runtime::RunResult result = causis::runtime::run_program(*program, 0, &log);
  ASSERT_TRUE(result.ok);

  EXPECT_EQ(log.frames.size(), 1U);
  EXPECT_EQ(log.frames[0].tick, 0);
  EXPECT_EQ(log.frames[0].robots[0].x, 0);
}

TEST(TickLog, WritesJsonFile) {
  causis::runtime::TickLog log;
  log.width = 2;
  log.height = 2;
  causis::runtime::TickFrame frame;
  frame.tick = 0;
  log.frames.push_back(frame);

  EXPECT_TRUE(causis::runtime::write_tick_log_file("build/test_tick_log.json", log));
}
