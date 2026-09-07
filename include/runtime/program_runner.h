#pragma once

#include "ast/ast.h"
#include "runtime/simulation.h"

#include <optional>
#include <string>
#include <stdexcept>

namespace causis::runtime {

// run_program() compiles the AST to optimized bytecode and executes it on the VM.
// The AST Executor remains in the tree for reference; production execution uses the VM.

struct RuntimeError {
    std::string message;
    int line{1};
    int column{1};
};

class RuntimeException : public std::runtime_error {
public:
    RuntimeException(RuntimeError error);

    const RuntimeError& error() const;

private:
    RuntimeError error_;
};

// file will compile and execute the AST Program via bytecode + VM
struct RunResult {
    bool ok{false};
    std::optional<Simulation> simulation;
    std::optional<RuntimeError> error;
};

RunResult run_program(const ast::Program& program, int tick_count);

std::string format_run_summary(const Simulation& simulation);

} // namespace causis::runtime
