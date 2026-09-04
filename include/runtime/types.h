#pragma once

namespace causis::runtime {

enum class Direction {  // robot face direction
    Up,
    Right,
    Down,
    Left,
};

enum class CellKind {
    Empty,
    Obstacle,
    Target,
    Robot,
};

struct Position {
    int x{0};
    int y{0};

    bool operator==(const Position& other) const = default;
};

Direction turn_left(Direction direction);
Direction turn_right(Direction direction);
Position direction_delta(Direction direction); //tells how to move one cell based on direction

} // namespace causis::runtime
