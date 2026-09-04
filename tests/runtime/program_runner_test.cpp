#include "lexer/lexer.h"
#include "parser/parser.h"
#include "runtime/program_runner.h"
#include "semantic/analyzer.h"

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

} // namespace
} // namespace causis

TEST(ProgramRunner, RunsBasicMoveExample) {
    const std::string source = causis::read_file("examples/basic_move.ls");
    ASSERT_FALSE(source.empty());

    const std::unique_ptr<causis::ast::Program> program = causis::compile_program(source);
    ASSERT_NE(program, nullptr);

    const causis::runtime::RunResult result = causis::runtime::run_program(*program, 7);
    ASSERT_TRUE(result.ok);
    ASSERT_TRUE(result.simulation.has_value());

    const causis::runtime::Robot& robot = result.simulation->world().robot("R");
    EXPECT_EQ(robot.x, 7);
    EXPECT_EQ(robot.y, 2);
}

TEST(ProgramRunner, RunsCollisionExampleDeterministically) {
    const std::string source = causis::read_file("examples/collision.ls");
    ASSERT_FALSE(source.empty());

    const std::unique_ptr<causis::ast::Program> program = causis::compile_program(source);
    ASSERT_NE(program, nullptr);

    const causis::runtime::RunResult first = causis::runtime::run_program(*program, 5);
    const causis::runtime::RunResult second = causis::runtime::run_program(*program, 5);
    ASSERT_TRUE(first.ok);
    ASSERT_TRUE(second.ok);

    const causis::runtime::Robot& a = first.simulation->world().robot("R");
    const causis::runtime::Robot& b = second.simulation->world().robot("R");
    EXPECT_EQ(a.x, b.x);
    EXPECT_EQ(a.y, b.y);
    EXPECT_EQ(a.direction, b.direction);
}

TEST(ProgramRunner, RunsTargetExample) {
    const std::string source = causis::read_file("examples/target.ls");
    ASSERT_FALSE(source.empty());

    const std::unique_ptr<causis::ast::Program> program = causis::compile_program(source);
    ASSERT_NE(program, nullptr);

    const causis::runtime::RunResult result = causis::runtime::run_program(*program, 3);
    ASSERT_TRUE(result.ok);

    const causis::runtime::Robot& robot = result.simulation->world().robot("R");
    EXPECT_GT(robot.x, 1);
    EXPECT_GT(robot.y, 1);
}

TEST(ProgramRunner, StopEndsRemainingStatementsInTick) {
    const std::string source = R"(
world 5 5;
robot R at 0 0;
behavior R {
    every tick {
        move_right();
        stop();
        move_right();
    }
}
)";

    const std::unique_ptr<causis::ast::Program> program = causis::compile_program(source);
    ASSERT_NE(program, nullptr);

    const causis::runtime::RunResult result = causis::runtime::run_program(*program, 1);
    ASSERT_TRUE(result.ok);

    const causis::runtime::Robot& robot = result.simulation->world().robot("R");
    EXPECT_EQ(robot.x, 1);
    EXPECT_EQ(robot.y, 0);
}

TEST(ProgramRunner, RunsRequestedNumberOfTicks) {
    const std::string source = causis::read_file("examples/basic_move.ls");
    ASSERT_FALSE(source.empty());

    const std::unique_ptr<causis::ast::Program> program = causis::compile_program(source);
    ASSERT_NE(program, nullptr);

    const causis::runtime::RunResult result = causis::runtime::run_program(*program, 3);
    ASSERT_TRUE(result.ok);
    EXPECT_EQ(result.simulation->tick_count(), 3);
}

TEST(ProgramRunner, ReportsDivisionByZero) {
    const std::string source = R"(
world 5 5;
robot R at 0 0;
behavior R {
    every tick {
        x = 1 / 0;
    }
}
)";

    const std::unique_ptr<causis::ast::Program> program = causis::compile_program(source);
    ASSERT_NE(program, nullptr);

    const causis::runtime::RunResult result = causis::runtime::run_program(*program, 1);
    ASSERT_FALSE(result.ok);
    ASSERT_TRUE(result.error.has_value());
    EXPECT_NE(result.error->message.find("division by zero"), std::string::npos);
}
