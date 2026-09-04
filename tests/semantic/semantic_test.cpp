#include "lexer/lexer.h"
#include "parser/parser.h"
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

semantic::SemanticResult analyze_source(const std::string& source) {
    lexer::Lexer lexer(source);
    const lexer::TokenizeResult tokens = lexer.tokenize();
    if (tokens.error.has_value()) {
        semantic::SemanticResult result;
        semantic::SemanticError err;
        err.message = tokens.error->message;
        err.line = tokens.error->line;
        err.column = tokens.error->column;
        result.error = err;
        return result;
    }

    parser::Parser parser(tokens.tokens);
    const parser::ParseResult parse_result = parser.parse_program();
    if (parse_result.error.has_value()) {
        semantic::SemanticResult result;
        semantic::SemanticError err;
        err.message = parse_result.error->message;
        err.line = parse_result.error->line;
        err.column = parse_result.error->column;
        result.error = err;
        return result;
    }

    semantic::Analyzer analyzer;
    return analyzer.analyze(*parse_result.program);
}

} // namespace
} // namespace causis

TEST(SemanticWorld, RequiresWorldDeclaration) {
    const causis::semantic::SemanticResult result = causis::analyze_source("robot R at 0 0;");
    ASSERT_TRUE(result.error.has_value());
    EXPECT_NE(result.error->message.find("world"), std::string::npos);
}

TEST(SemanticWorld, RejectsDuplicateWorld) {
    const causis::semantic::SemanticResult result =
        causis::analyze_source("world 5 5;\nworld 10 10;");
    ASSERT_TRUE(result.error.has_value());
    EXPECT_NE(result.error->message.find("one world"), std::string::npos);
}

TEST(SemanticWorld, RejectsNonPositiveDimensions) {
    const causis::semantic::SemanticResult result = causis::analyze_source("world 0 5;");
    ASSERT_TRUE(result.error.has_value());
}

TEST(SemanticEntities, RejectsDuplicateRobotNames) {
    const causis::semantic::SemanticResult result = causis::analyze_source(R"(
world 5 5;
robot R at 0 0;
robot R at 1 1;
)");
    ASSERT_TRUE(result.error.has_value());
    EXPECT_NE(result.error->message.find("duplicate robot"), std::string::npos);
}

TEST(SemanticEntities, RejectsOverlappingEntities) {
    const causis::semantic::SemanticResult result = causis::analyze_source(R"(
world 5 5;
robot R at 1 1;
target T at 1 1;
)");
    ASSERT_TRUE(result.error.has_value());
    EXPECT_NE(result.error->message.find("overlaps"), std::string::npos);
}

TEST(SemanticEntities, RejectsOutOfBoundsCoordinates) {
    const causis::semantic::SemanticResult result = causis::analyze_source(R"(
world 5 5;
robot R at 9 0;
)");
    ASSERT_TRUE(result.error.has_value());
    EXPECT_NE(result.error->message.find("outside world"), std::string::npos);
}

TEST(SemanticBehavior, RejectsUnknownRobot) {
    const causis::semantic::SemanticResult result = causis::analyze_source(R"(
world 5 5;
behavior R {
    every tick {
        move_right();
    }
}
)");
    ASSERT_TRUE(result.error.has_value());
    EXPECT_NE(result.error->message.find("unknown robot"), std::string::npos);
}

TEST(SemanticBehavior, RejectsUnknownTarget) {
    const causis::semantic::SemanticResult result = causis::analyze_source(R"(
world 5 5;
robot R at 0 0;
behavior R {
    every tick {
        move_toward(T);
    }
}
)");
    ASSERT_TRUE(result.error.has_value());
    EXPECT_NE(result.error->message.find("unknown target"), std::string::npos);
}

TEST(SemanticBehavior, RejectsNonBooleanIfCondition) {
    const causis::semantic::SemanticResult result = causis::analyze_source(R"(
world 5 5;
robot R at 0 0;
behavior R {
    every tick {
        if 1 + 2 {
            move_right();
        }
    }
}
)");
    ASSERT_TRUE(result.error.has_value());
    EXPECT_NE(result.error->message.find("boolean"), std::string::npos);
}

TEST(SemanticVariables, AcceptsAssignmentStatement) {
    const causis::semantic::SemanticResult result = causis::analyze_source(R"(
world 5 5;
robot R at 0 0;
behavior R {
    every tick {
        speed = 2;
        move_right();
    }
}
)");
    ASSERT_FALSE(result.error.has_value());
    EXPECT_TRUE(result.ok);
}

TEST(SemanticVariables, RejectsUnknownVariableRead) {
    const causis::semantic::SemanticResult result = causis::analyze_source(R"(
world 5 5;
robot R at 0 0;
behavior R {
    every tick {
        speed = speed + 1;
    }
}
)");
    ASSERT_TRUE(result.error.has_value());
    EXPECT_NE(result.error->message.find("unknown variable"), std::string::npos);
}

TEST(SemanticBuiltins, RejectsUnknownFunction) {
    const causis::semantic::SemanticResult result = causis::analyze_source(R"(
world 5 5;
robot R at 0 0;
behavior R {
    every tick {
        jump();
    }
}
)");
    ASSERT_TRUE(result.error.has_value());
    EXPECT_NE(result.error->message.find("unknown function"), std::string::npos);
}

TEST(SemanticExamples, AcceptsBasicMoveFile) {
    const std::string source = causis::read_file("examples/basic_move.ls");
    ASSERT_FALSE(source.empty());

    const causis::semantic::SemanticResult result = causis::analyze_source(source);
    ASSERT_FALSE(result.error.has_value());
    EXPECT_TRUE(result.ok);
}

TEST(SemanticExamples, AcceptsCollisionFile) {
    const std::string source = causis::read_file("examples/collision.ls");
    ASSERT_FALSE(source.empty());

    const causis::semantic::SemanticResult result = causis::analyze_source(source);
    ASSERT_FALSE(result.error.has_value());
    EXPECT_TRUE(result.ok);
}

TEST(SemanticExamples, AcceptsTargetFile) {
    const std::string source = causis::read_file("examples/target.ls");
    ASSERT_FALSE(source.empty());

    const causis::semantic::SemanticResult result = causis::analyze_source(source);
    ASSERT_FALSE(result.error.has_value());
    EXPECT_TRUE(result.ok);
}
