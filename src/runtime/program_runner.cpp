#include "runtime/program_runner.h"

#include "runtime/executor.h"

#include <sstream>
#include <unordered_map>

// take AST and make it run for some number of ticks


namespace causis::runtime {
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

// take the AST and build the world
World build_world(const ast::Program& program) {
    const ast::WorldDecl* world_decl = nullptr;
    for (const std::unique_ptr<ast::Decl>& decl : program.declarations) {
        if (const auto* world = dynamic_cast<const ast::WorldDecl*>(decl.get())) {
            world_decl = world;
            break;
        }
    }

    if (world_decl == nullptr) {
        RuntimeError err;
        err.message = "program must declare a world";
        throw RuntimeException(std::move(err));
    }

    World world(world_decl->width, world_decl->height);

    for (const std::unique_ptr<ast::Decl>& decl : program.declarations) {
        if (const auto* robot = dynamic_cast<const ast::RobotDecl*>(decl.get())) {
            if (!world.place_robot(robot->name, robot->x, robot->y)) {
                RuntimeError err;
                err.message = "failed to place robot '" + robot->name + "'";
                err.line = robot->line;
                err.column = robot->column;
                throw RuntimeException(std::move(err));
            }
            continue;
        }

        if (const auto* target = dynamic_cast<const ast::TargetDecl*>(decl.get())) {
            if (!world.place_target(target->name, target->x, target->y)) {
                RuntimeError err;
                err.message = "failed to place target '" + target->name + "'";
                err.line = target->line;
                err.column = target->column;
                throw RuntimeException(std::move(err));
            }
            continue;
        }

        if (const auto* obstacle = dynamic_cast<const ast::ObstacleDecl*>(decl.get())) {
            if (!world.place_obstacle(obstacle->x, obstacle->y)) {
                RuntimeError err;
                err.message = "failed to place obstacle";
                err.line = obstacle->line;
                err.column = obstacle->column;
                throw RuntimeException(std::move(err));
            }
        }
    }

    return world;
}

} // namespace

RuntimeException::RuntimeException(RuntimeError error)
    : std::runtime_error(error.message), error_(std::move(error)) {}

const RuntimeError& RuntimeException::error() const {
    return error_;
}

RunResult run_program(const ast::Program& program, int tick_count) {
    RunResult result;

    if (tick_count < 0) {
        RuntimeError err;
        err.message = "tick count must be non-negative";
        result.error = err;
        return result;
    }

    try {
        Simulation simulation(build_world(program));

        std::vector<std::string> robot_order;
        std::unordered_map<std::string, const ast::BehaviorDecl*> behaviors;

        for (const std::unique_ptr<ast::Decl>& decl : program.declarations) {
            if (const auto* robot = dynamic_cast<const ast::RobotDecl*>(decl.get())) {
                robot_order.push_back(robot->name);
                continue;
            }

            if (const auto* behavior = dynamic_cast<const ast::BehaviorDecl*>(decl.get())) {
                behaviors[behavior->robot_name] = behavior;
            }
        }

        for (int tick = 0; tick < tick_count; ++tick) {
            simulation.begin_tick();

            for (const std::string& robot_name : robot_order) {
                const auto found = behaviors.find(robot_name);
                if (found == behaviors.end()) {
                    continue;
                }

                Executor executor(simulation, robot_name);
                for (const std::unique_ptr<ast::EveryTickStmt>& event : found->second->event_blocks) {
                    if (event->body != nullptr) {
                        executor.execute_block(*event->body);
                    }
                }
            }

            simulation.end_tick();
        }

        result.simulation = std::move(simulation);
        result.ok = true;
        return result;
    } catch (const RuntimeException& ex) {
        result.error = ex.error();
        return result;
    }
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
