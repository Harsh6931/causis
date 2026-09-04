#include "runtime/types.h"

namespace causis::runtime {

Direction turn_left(Direction direction) {
    switch (direction) {
    case Direction::Up:
        return Direction::Left;
    case Direction::Left:
        return Direction::Down;
    case Direction::Down:
        return Direction::Right;
    case Direction::Right:
        return Direction::Up;
    }

    return Direction::Right;
}

Direction turn_right(Direction direction) {
    switch (direction) {
    case Direction::Up:
        return Direction::Right;
    case Direction::Right:
        return Direction::Down;
    case Direction::Down:
        return Direction::Left;
    case Direction::Left:
        return Direction::Up;
    }

    return Direction::Right;
}


// assuming (0,0) is at the top left corner of the grid

// delta is how much change needed to move one cell based on direction

Position direction_delta(Direction direction) {
    switch (direction) {
    case Direction::Up:  // x increase to right, y decreases downward
        return {0, -1};
    case Direction::Right:
        return {1, 0};
    case Direction::Down:
        return {0, 1};
    case Direction::Left:
        return {-1, 0};
    }

    return {0, 0};
}

} // namespace causis::runtime
