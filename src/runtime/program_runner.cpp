#include "runtime/program_runner.h"

#include "bytecode/compiler.h"
#include "ir/lower.h"
#include "ir/optimizer.h"
#include "vm/vm.h"

#include <sstream>

namespace causis::runtime {

RuntimeException::RuntimeException(RuntimeError error)
    : std::runtime_error(error.message), error_(std::move(error)) {}

const RuntimeError& RuntimeException::error() const {
    return error_;
}

namespace {

std::string direction_name(Direction direction) {
    switch (direction) {
    case Direction::Up:
        return "up";
    case Direction::Right:
        return "right";
    case Direction::Down:
        return "down";
    case Direction::Left:
        return "left";
    }

    return "right";
}

} // namespace

RunResult run_program(const ast::Program& program, int tick_count) {
    RunResult result;

    if (tick_count < 0) {
        RuntimeError err;
        err.message = "tick count must be non-negative";
        result.error = err;
        return result;
    }

    const ir::LowerResult lower_result = ir::lower_program(program);
    if (lower_result.error.has_value()) {
        RuntimeError err;
        err.message = lower_result.error->message;
        err.line = lower_result.error->line;
        err.column = lower_result.error->column;
        result.error = err;
        return result;
    }

    if (!lower_result.program.has_value()) {
        RuntimeError err;
        err.message = "IR lowering produced no program";
        result.error = err;
        return result;
    }

    const ir::IrProgram optimized = ir::optimize_program(*lower_result.program);
    const bytecode::CompileResult compile_result = bytecode::compile_ir(optimized);
    if (compile_result.error.has_value()) {
        RuntimeError err;
        err.message = compile_result.error->message;
        err.line = compile_result.error->line;
        err.column = compile_result.error->column;
        result.error = err;
        return result;
    }

    if (!compile_result.program.has_value()) {
        RuntimeError err;
        err.message = "bytecode compilation produced no program";
        result.error = err;
        return result;
    }

    const vm::VmResult vm_result = vm::run_bytecode(*compile_result.program, tick_count);
    if (vm_result.error.has_value()) {
        RuntimeError err;
        err.message = vm_result.error->message;
        err.line = vm_result.error->line;
        err.column = vm_result.error->column;
        result.error = err;
        return result;
    }

    if (!vm_result.simulation.has_value()) {
        RuntimeError err;
        err.message = "VM did not produce a simulation";
        result.error = err;
        return result;
    }

    result.ok = true;
    result.simulation = std::move(*vm_result.simulation);
    return result;
}

std::string format_run_summary(const Simulation& simulation) {
    std::ostringstream out;
    out << "Simulation complete (" << simulation.tick_count() << " ticks).\n";

    for (const Robot& robot : simulation.world().robots()) {
        out << "Robot " << robot.name << ": position (" << robot.x << ", " << robot.y
            << "), direction " << direction_name(robot.direction) << ", collision "
            << (robot.collision_flag ? "true" : "false") << '\n';
    }

    return out.str();
}

} // namespace causis::runtime
