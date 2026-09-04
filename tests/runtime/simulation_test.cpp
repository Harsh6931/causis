#include "runtime/simulation.h"
#include "runtime/types.h"
#include "runtime/world.h"

#include <gtest/gtest.h>

namespace causis::runtime {
namespace {

World make_basic_world() {
    World world(8, 5);
    EXPECT_TRUE(world.place_robot("R", 0, 2));
    return world;
}

} // namespace
} // namespace causis::runtime

TEST(SimulationMovement, MovesRobotRight) {
    causis::runtime::World world = causis::runtime::make_basic_world();

    EXPECT_TRUE(world.move_right("R"));
    EXPECT_EQ(world.robot("R").x, 1);
    EXPECT_EQ(world.robot("R").y, 2);
    EXPECT_FALSE(world.collision("R"));
}

TEST(SimulationMovement, BlocksMoveIntoWall) {
    causis::runtime::World world(3, 3);
    ASSERT_TRUE(world.place_robot("R", 2, 1));

    EXPECT_FALSE(world.move_right("R"));
    EXPECT_EQ(world.robot("R").x, 2);
    EXPECT_TRUE(world.collision("R"));
}

TEST(SimulationMovement, BlocksMoveIntoObstacle) {
    causis::runtime::World world(5, 5);
    ASSERT_TRUE(world.place_robot("R", 1, 2));
    ASSERT_TRUE(world.place_obstacle(2, 2));

    EXPECT_FALSE(world.move_right("R"));
    EXPECT_EQ(world.robot("R").x, 1);
    EXPECT_TRUE(world.collision("R"));
}

TEST(SimulationMovement, BlocksMoveIntoTarget) {
    causis::runtime::World world(5, 5);
    ASSERT_TRUE(world.place_robot("R", 1, 2));
    ASSERT_TRUE(world.place_target("T", 2, 2));

    EXPECT_FALSE(world.move_right("R"));
    EXPECT_EQ(world.robot("R").x, 1);
    EXPECT_TRUE(world.collision("R"));
}

TEST(SimulationMovement, MovesForwardUsingDirection) {
    causis::runtime::World world(5, 5);
    ASSERT_TRUE(world.place_robot("R", 1, 2));

    EXPECT_TRUE(world.move_forward("R"));
    EXPECT_EQ(world.robot("R").x, 2);
    EXPECT_EQ(world.robot("R").direction, causis::runtime::Direction::Right);
}

TEST(SimulationMovement, TurnsAndMovesForward) {
    causis::runtime::World world(5, 5);
    ASSERT_TRUE(world.place_robot("R", 1, 2));

    world.turn_left("R");
    EXPECT_EQ(world.robot("R").direction, causis::runtime::Direction::Up);

    EXPECT_TRUE(world.move_forward("R"));
    EXPECT_EQ(world.robot("R").x, 1);
    EXPECT_EQ(world.robot("R").y, 1);
}

TEST(SimulationMovement, MoveTowardPrefersXAxisWhenEqualDistance) {
    causis::runtime::World world(6, 6);
    ASSERT_TRUE(world.place_robot("R", 1, 1));
    ASSERT_TRUE(world.place_target("T", 2, 2));

    EXPECT_TRUE(world.move_toward("R", "T"));
    EXPECT_EQ(world.robot("R").x, 2);
    EXPECT_EQ(world.robot("R").y, 1);
}

TEST(SimulationQueries, ComputesManhattanDistance) {
    causis::runtime::World world(10, 10);
    ASSERT_TRUE(world.place_robot("R", 1, 1));
    ASSERT_TRUE(world.place_target("T", 4, 5));

    EXPECT_EQ(world.distance_to("R", "T"), 7);
}

TEST(SimulationQueries, DetectsObstacleAhead) {
    causis::runtime::World world(6, 6);
    ASSERT_TRUE(world.place_robot("R", 1, 2));
    ASSERT_TRUE(world.place_obstacle(2, 2));

    EXPECT_TRUE(world.obstacle_ahead("R"));
}

TEST(SimulationQueries, DetectsWorldEdgeAhead) {
    causis::runtime::World world(3, 3);
    ASSERT_TRUE(world.place_robot("R", 2, 1));

    EXPECT_TRUE(world.obstacle_ahead("R"));
}

TEST(SimulationQueries, IgnoresTargetsAndRobotsForObstacleAhead) {
    causis::runtime::World world(6, 6);
    ASSERT_TRUE(world.place_robot("R", 1, 2));
    ASSERT_TRUE(world.place_target("T", 2, 2));

    EXPECT_FALSE(world.obstacle_ahead("R"));
}

TEST(SimulationTick, ClearsCollisionFlagsOnBeginTick) {
    causis::runtime::World world(8, 5);
    ASSERT_TRUE(world.place_robot("R", 7, 2));
    causis::runtime::Simulation simulation(std::move(world));

    EXPECT_FALSE(simulation.world().move_right("R"));
    EXPECT_TRUE(simulation.world().collision("R"));

    simulation.begin_tick();
    EXPECT_FALSE(simulation.world().collision("R"));
}

TEST(SimulationTick, IncrementsTickCount) {
    causis::runtime::Simulation simulation(causis::runtime::make_basic_world());

    EXPECT_EQ(simulation.tick_count(), 0);
    simulation.tick();
    EXPECT_EQ(simulation.tick_count(), 1);
}

TEST(SimulationReset, RestoresInitialState) {
    causis::runtime::Simulation simulation(causis::runtime::make_basic_world());

    simulation.world().move_right("R");
    simulation.tick();
    simulation.reset();

    EXPECT_EQ(simulation.tick_count(), 0);
    EXPECT_EQ(simulation.world().robot("R").x, 0);
    EXPECT_EQ(simulation.world().robot("R").y, 2);
    EXPECT_FALSE(simulation.world().collision("R"));
}

TEST(SimulationDeterminism, RepeatsSameMoves) {
    causis::runtime::World first(5, 5);
    causis::runtime::World second(5, 5);
    ASSERT_TRUE(first.place_robot("R", 1, 2));
    ASSERT_TRUE(second.place_robot("R", 1, 2));
    ASSERT_TRUE(first.place_obstacle(3, 2));
    ASSERT_TRUE(second.place_obstacle(3, 2));

    for (int i = 0; i < 3; ++i) {
        first.move_right("R");
        second.move_right("R");
    }

    EXPECT_EQ(first.robot("R").x, second.robot("R").x);
    EXPECT_EQ(first.robot("R").y, second.robot("R").y);
    EXPECT_EQ(first.collision("R"), second.collision("R"));
}

TEST(SimulationGrid, CellAtRejectsOutOfBoundsCoordinates) {
    causis::runtime::World world(3, 3);

    EXPECT_THROW(world.cell_at(-1, 0), std::out_of_range);
    EXPECT_THROW(world.cell_at(0, -1), std::out_of_range);
    EXPECT_THROW(world.cell_at(3, 0), std::out_of_range);
    EXPECT_THROW(world.cell_at(0, 3), std::out_of_range);
}

TEST(SimulationTypes, RotatesDirections) {
    EXPECT_EQ(causis::runtime::turn_left(causis::runtime::Direction::Up),
              causis::runtime::Direction::Left);
    EXPECT_EQ(causis::runtime::turn_right(causis::runtime::Direction::Up),
              causis::runtime::Direction::Right);
}
