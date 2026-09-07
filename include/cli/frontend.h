#pragma once

#include "ast/ast.h"

#include <memory>
#include <optional>
#include <string>

namespace causis::cli {

enum class FrontendStage { Io, Lexer, Parser, Semantic };

struct FrontendError {
  FrontendStage stage{FrontendStage::Io};
  std::string message;
  int line{1};
  int column{1};
};

struct FrontendResult {
  bool ok{false};
  std::unique_ptr<ast::Program> program;
  std::optional<FrontendError> error;
};

// Read a .ls file and run lexer + parser.
FrontendResult parse_program_file(const std::string& path);

// Read a .ls file and run lexer, parser, and semantic analysis.
FrontendResult load_program(const std::string& path);

void print_frontend_error(const FrontendError& error);

} // namespace causis::cli
