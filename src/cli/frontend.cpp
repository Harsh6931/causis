#include "cli/frontend.h"

#include "lexer/lexer.h"
#include "parser/parser.h"
#include "semantic/analyzer.h"

#include <fstream>
#include <iostream>
#include <sstream>

namespace causis::cli {
namespace {

FrontendResult make_error(FrontendStage stage, const std::string& message, int line = 1,
                          int column = 1) {
  FrontendResult result;
  FrontendError err;
  err.stage = stage;
  err.message = message;
  err.line = line;
  err.column = column;
  result.error = err;
  return result;
}

bool read_source_file(const std::string& path, std::string& source) {
  std::ifstream file(path);
  if (!file) {
    return false;
  }

  std::ostringstream buffer;
  buffer << file.rdbuf();
  source = buffer.str();
  return true;
}

const char* stage_name(FrontendStage stage) {
  switch (stage) {
  case FrontendStage::Io:
    return "I/O";
  case FrontendStage::Lexer:
    return "Lexer";
  case FrontendStage::Parser:
    return "Parser";
  case FrontendStage::Semantic:
    return "Semantic";
  }

  return "Frontend";
}

} // namespace

void print_frontend_error(const FrontendError& error) {
  if (error.stage == FrontendStage::Io || error.stage == FrontendStage::Lexer) {
    std::cerr << stage_name(error.stage) << " error: " << error.message << '\n';
    return;
  }

  std::cerr << stage_name(error.stage) << " error: " << error.message << " at line "
            << error.line << ", column " << error.column << '\n';
}

FrontendResult parse_program_file(const std::string& path) {
  std::string source;
  if (!read_source_file(path, source)) {
    return make_error(FrontendStage::Io, "could not open file '" + path + "'");
  }

  lexer::Lexer lexer(source);
  const lexer::TokenizeResult lex_result = lexer.tokenize();
  if (lex_result.error.has_value()) {
    return make_error(FrontendStage::Lexer, lex_result.error->message, lex_result.error->line,
                      lex_result.error->column);
  }

  parser::Parser parser(lex_result.tokens);
  parser::ParseResult parse_result = parser.parse_program();
  if (parse_result.error.has_value()) {
    return make_error(FrontendStage::Parser, parse_result.error->message,
                      parse_result.error->line, parse_result.error->column);
  }

  FrontendResult result;
  result.ok = true;
  result.program = std::move(parse_result.program);
  return result;
}

FrontendResult load_program(const std::string& path) {
  FrontendResult parsed = parse_program_file(path);
  if (!parsed.ok) {
    return parsed;
  }

  semantic::Analyzer analyzer;
  const semantic::SemanticResult semantic_result = analyzer.analyze(*parsed.program);
  if (semantic_result.error.has_value()) {
    return make_error(FrontendStage::Semantic, semantic_result.error->message,
                      semantic_result.error->line, semantic_result.error->column);
  }

  return parsed;
}

} // namespace causis::cli
