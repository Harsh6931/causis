#include "runtime/simulation.h"

namespace causis::runtime {

Simulation::Simulation(World world) : initial_world_(std::move(world)), world_(initial_world_) {}

World& Simulation::world() {
    return world_;
}

const World& Simulation::world() const {
    return world_;
}

int Simulation::tick_count() const {
    return tick_;
}

// beginning of each tick, clear collision flags
void Simulation::begin_tick() {
    world_.clear_collision_flags();
}

// advance to the next tick after one ends
void Simulation::end_tick() {
    tick_++;
}

void Simulation::tick() {
    begin_tick();
    end_tick();
}

void Simulation::reset() {
    world_ = initial_world_;
    tick_ = 0;
}

} // namespace causis::runtime
