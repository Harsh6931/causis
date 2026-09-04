#pragma once

#include "runtime/world.h"

namespace causis::runtime {

    // Simulation is created from world

    // 2 world are maintained 
        // 1. initial_world_ is the world at the start of the simulation
        // 2. world_ is the world at the current that change during simulation

        // Used 2 copy for reset() to restore initial state
class Simulation {
public:
    explicit Simulation(World world);

    World& world();
    const World& world() const;

    int tick_count() const;

    void begin_tick();
    void end_tick();
    void tick();

    void reset();

private:
    World initial_world_;
    World world_;
    int tick_{0};
};

} // namespace causis::runtime
