#pragma once

#include "ast/ast.h"
#include "runtime/simulation.h"

#include <optional>
#include <string>
#include <stdexcept>

namespace causis::runtime {

// Stage 6 bridge: run_program() walks the AST via Executor and drives World/Simulation
// directly. This validates simulation semantics before IR/bytecode/VM exist (Stages 7-10).
// The final pipeline is AST -> IR -> bytecode -> VM; the VM will replace this interpreter.

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

// file will execute the AST Program and return the result of the simulation
// (interim path — not the long-term execution model; see comment above)
struct RunResult {
    bool ok{false};
    std::optional<Simulation> simulation;
    std::optional<RuntimeError> error;
};

RunResult run_program(const ast::Program& program, int tick_count);

std::string format_run_summary(const Simulation& simulation);

} // namespace causis::runtime
