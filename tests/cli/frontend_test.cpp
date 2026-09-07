#include "cli/frontend.h"

#include <gtest/gtest.h>

TEST(CliFrontend, ParsesExampleProgram) {
  const causis::cli::FrontendResult result =
      causis::cli::parse_program_file("examples/basic_move.ls");
  ASSERT_TRUE(result.ok);
  ASSERT_NE(result.program, nullptr);
}

TEST(CliFrontend, LoadsSemanticallyValidProgram) {
  const causis::cli::FrontendResult result =
      causis::cli::load_program("examples/basic_move.ls");
  ASSERT_TRUE(result.ok);
  ASSERT_NE(result.program, nullptr);
}

TEST(CliFrontend, ReportsMissingFile) {
  const causis::cli::FrontendResult result =
      causis::cli::load_program("examples/does_not_exist.ls");
  ASSERT_FALSE(result.ok);
  ASSERT_TRUE(result.error.has_value());
  EXPECT_EQ(result.error->stage, causis::cli::FrontendStage::Io);
}
