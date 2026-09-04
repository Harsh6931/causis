#include "ast/ast.h"
#include "lexer/lexer.h"
#include "parser/parser.h"

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

parser::ParseResult parse_source(const std::string& source) {
    lexer::Lexer lexer(source);
    const lexer::TokenizeResult tokens = lexer.tokenize();
    if (tokens.error.has_value()) {
        parser::ParseResult result;
        parser::ParseError err;
        err.message = tokens.error->message;
        err.line = tokens.error->line;
        err.column = tokens.error->column;
        result.error = err;
        return result;
    }

    parser::Parser parser(tokens.tokens);
    return parser.parse_program();
}

} // namespace
} // namespace causis

TEST(ParserWorld, ParsesWorldDeclaration) {
    const causis::parser::ParseResult result = causis::parse_source("world 10 20;");
    ASSERT_FALSE(result.error.has_value());
    ASSERT_EQ(result.program->declarations.size(), 1U);

    const auto* world = dynamic_cast<const causis::ast::WorldDecl*>(result.program->declarations[0].get());
    ASSERT_NE(world, nullptr);
    EXPECT_EQ(world->width, 10);
    EXPECT_EQ(world->height, 20);
}

TEST(ParserRobot, ParsesRobotDeclaration) {
    const causis::parser::ParseResult result = causis::parse_source("robot R at 1 2;");
    ASSERT_FALSE(result.error.has_value());

    const auto* robot = dynamic_cast<const causis::ast::RobotDecl*>(result.program->declarations[0].get());
    ASSERT_NE(robot, nullptr);
    EXPECT_EQ(robot->name, "R");
    EXPECT_EQ(robot->x, 1);
    EXPECT_EQ(robot->y, 2);
}

TEST(ParserBehavior, ParsesEveryTickCall) {
    const std::string source = R"(
behavior R {
    every tick {
        move_right();
    }
}
)";

    const causis::parser::ParseResult result = causis::parse_source(source);
    ASSERT_FALSE(result.error.has_value());

    const auto* behavior = dynamic_cast<const causis::ast::BehaviorDecl*>(result.program->declarations[0].get());
    ASSERT_NE(behavior, nullptr);
    ASSERT_EQ(behavior->event_blocks.size(), 1U);

    const auto* every_tick = behavior->event_blocks[0].get();
    ASSERT_NE(every_tick, nullptr);
    ASSERT_EQ(every_tick->body->statements.size(), 1U);

    const auto* expr_stmt = dynamic_cast<const causis::ast::ExprStmt*>(every_tick->body->statements[0].get());
    ASSERT_NE(expr_stmt, nullptr);

    const auto* call = dynamic_cast<const causis::ast::CallExpr*>(expr_stmt->expression.get());
    ASSERT_NE(call, nullptr);
    EXPECT_EQ(call->callee, "move_right");
    EXPECT_TRUE(call->arguments.empty());
}

TEST(ParserPrecedence, MultiplicationBeforeAddition) {
    const std::string source = R"(
behavior R {
    every tick {
        if 1 + 2 * 3 == 7 {
            move_right();
        }
    }
}
)";

    const causis::parser::ParseResult result = causis::parse_source(source);
    ASSERT_FALSE(result.error.has_value());

    const auto* behavior = dynamic_cast<const causis::ast::BehaviorDecl*>(result.program->declarations[0].get());
    const auto* every_tick = behavior->event_blocks[0].get();
    const auto* if_stmt = dynamic_cast<const causis::ast::IfStmt*>(every_tick->body->statements[0].get());
    ASSERT_NE(if_stmt, nullptr);

    const auto* equality = dynamic_cast<const causis::ast::BinaryExpr*>(if_stmt->condition.get());
    ASSERT_NE(equality, nullptr);
    EXPECT_EQ(equality->op, causis::lexer::TokenType::EqualEqual);

    const auto* addition = dynamic_cast<const causis::ast::BinaryExpr*>(equality->left.get());
    ASSERT_NE(addition, nullptr);
    EXPECT_EQ(addition->op, causis::lexer::TokenType::Plus);

    const auto* multiplication = dynamic_cast<const causis::ast::BinaryExpr*>(addition->right.get());
    ASSERT_NE(multiplication, nullptr);
    EXPECT_EQ(multiplication->op, causis::lexer::TokenType::Star);
}

TEST(ParserIf, ParsesIfElse) {
    const std::string source = R"(
behavior R {
    every tick {
        if obstacle_ahead() {
            turn_right();
        } else {
            move_forward();
        }
    }
}
)";

    const causis::parser::ParseResult result = causis::parse_source(source);
    ASSERT_FALSE(result.error.has_value());

    const auto* behavior = dynamic_cast<const causis::ast::BehaviorDecl*>(result.program->declarations[0].get());
    const auto* every_tick = behavior->event_blocks[0].get();
    const auto* if_stmt = dynamic_cast<const causis::ast::IfStmt*>(every_tick->body->statements[0].get());

    ASSERT_NE(if_stmt, nullptr);
    ASSERT_NE(if_stmt->else_branch, nullptr);
}

TEST(ParserExamples, ParsesBasicMoveFile) {
    const std::string source = causis::read_file("examples/basic_move.ls");
    ASSERT_FALSE(source.empty());

    const causis::parser::ParseResult result = causis::parse_source(source);
    ASSERT_FALSE(result.error.has_value());
    EXPECT_EQ(result.program->declarations.size(), 3U);
}

TEST(ParserExamples, ParsesCollisionFile) {
    const std::string source = causis::read_file("examples/collision.ls");
    ASSERT_FALSE(source.empty());

    const causis::parser::ParseResult result = causis::parse_source(source);
    ASSERT_FALSE(result.error.has_value());
    EXPECT_GE(result.program->declarations.size(), 3U);
}

TEST(ParserExamples, ParsesTargetFile) {
    const std::string source = causis::read_file("examples/target.ls");
    ASSERT_FALSE(source.empty());

    const causis::parser::ParseResult result = causis::parse_source(source);
    ASSERT_FALSE(result.error.has_value());

    const auto* target = dynamic_cast<const causis::ast::TargetDecl*>(result.program->declarations[2].get());
    ASSERT_NE(target, nullptr);
    EXPECT_EQ(target->name, "T");
}

TEST(ParserErrors, ReportsMissingSemicolon) {
    const causis::parser::ParseResult result = causis::parse_source("world 10 20");
    ASSERT_TRUE(result.error.has_value());
}
