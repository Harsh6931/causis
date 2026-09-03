# causis Semantics v1

> This is the canonical v1 spec; see [PLAN.md](../PLAN.md) Section 0 for project scope.

This document defines what valid causis v1 programs mean at compile time and
runtime.

---

## 1. World

A program must declare exactly one world:

```causis
world 20 20;
```

The first integer is width. The second integer is height.

Rules:

- width and height must be greater than zero
- valid x coordinates are `0` through `width - 1`
- valid y coordinates are `0` through `height - 1`
- `(0, 0)` is the top-left cell
- x increases to the right
- y increases downward

---

## 2. Occupancy

Each grid cell may contain at most one occupying entity.

Occupying entities:

```text
robot
target
obstacle
```

Two entities cannot be declared at the same coordinate in v1.

---

## 3. Declaration Order

The world declaration must appear before all entity and behavior declarations.

Robots and targets must be declared before they are referenced by behaviors.

Robot behavior execution order is robot declaration order.

---

## 4. Robots

Robots are movable agents.

Each robot has:

- a unique name
- a position
- a direction
- a collision flag
- zero or one behavior block

The default starting direction is `right`.

At the start of each tick, the robot's collision flag is cleared. If the robot
attempts an invalid movement during that tick, the flag becomes true.

---

## 5. Targets

Targets are named destination cells.

Targets do not move in v1.

Robots may query or move toward targets by name.

---

## 6. Obstacles

Obstacles are static blocked cells.

Obstacles are unnamed in v1.

Robots cannot move into obstacle cells.

---

## 7. Ticks

Simulation time advances in integer ticks.

For each tick:

1. clear each robot's collision flag
2. execute each robot's `every tick` block in robot declaration order
3. apply movement immediately as statements execute
4. increment the global tick counter after all robot behaviors finish

This makes execution deterministic.

---

## 8. Behavior Blocks

A behavior block attaches repeated logic to a declared robot.

```causis
behavior R {
    every tick {
        move_right();
    }
}
```

Inside a behavior, built-in action and query calls operate on the behavior's
own robot unless an argument explicitly names another entity.

---

## 9. Movement

Movement attempts compute a destination cell.

If the destination is inside the world and unoccupied by a robot, target, or
obstacle, the robot moves there.

If the destination is outside the world or occupied, the robot does not move
and its collision flag becomes true.

Movement into a target cell is blocked in v1 because targets occupy cells.
Reaching a target is represented by becoming adjacent to it or by later v2
rules; pathfinding and target completion are outside v1.

---

## 10. Directions

Directions are:

```text
up
right
down
left
```

`turn_left()` rotates counterclockwise:

```text
up -> left -> down -> right -> up
```

`turn_right()` rotates clockwise:

```text
up -> right -> down -> left -> up
```

---

## 11. Built-in Actions

### `move_up()`

Attempts to move the robot to `(x, y - 1)`.

### `move_down()`

Attempts to move the robot to `(x, y + 1)`.

### `move_left()`

Attempts to move the robot to `(x - 1, y)`.

### `move_right()`

Attempts to move the robot to `(x + 1, y)`.

### `move_forward()`

Attempts to move one cell in the robot's current direction.

### `move_toward(target)`

Attempts to move one cell closer to the named target.

v1 chooses the axis with the larger absolute distance first. If both axes are
equally far, x-axis movement is preferred. If the preferred move fails, v1 does
not automatically try the other axis.

### `turn_left()`

Rotates the robot left without changing position.

### `turn_right()`

Rotates the robot right without changing position.

### `stop()`

Ends the current robot's behavior for this tick. Other robots still execute
their behaviors.

---

## 12. Built-in Queries

### `distance_to(target)`

Returns the Manhattan distance from the robot to the named target:

```text
abs(robot.x - target.x) + abs(robot.y - target.y)
```

### `obstacle_ahead()`

Returns `true` if the cell directly ahead of the robot is outside the world or
occupied by an obstacle. Robots and targets do not count as obstacles for this
query in v1.

### `collision()`

Returns the robot's current collision flag.

---

## 13. Conditions

An `if` condition must evaluate to a boolean.

```causis
if obstacle_ahead() {
    turn_right();
}
```

If the condition is true, the first block executes.
If the condition is false and an `else` block exists, the `else` block executes.

---

## 14. Variables

Variables are local to a behavior.

Variables are created by assignment:

```causis
speed = 2;
```

A variable must be assigned before it is read.

Variables may hold:

```text
integer
boolean
```

Arithmetic expressions produce integers. The `/` operator uses integer
division (truncates toward zero).

v1 has type checking during semantic analysis where possible and at runtime
where necessary.

---

## 15. Compile-Time Semantic Errors

The compiler rejects:

- missing world declaration
- more than one world declaration
- non-positive world dimensions
- entity declaration before `world`
- duplicate robot names
- duplicate target names
- duplicate entity positions
- coordinates outside the world
- behavior for an unknown robot
- more than one behavior for the same robot
- unknown target references
- unknown variable reads
- unknown function calls
- wrong built-in argument count
- invalid argument kinds (e.g. non-integer where an integer is required)
- non-boolean `if` conditions

---

## 16. Runtime Errors

Runtime errors should be rare after semantic analysis.

Possible v1 runtime errors include:

- division by zero
- reading a variable that was not initialized due to control flow
- internal VM stack errors

Runtime errors stop execution and report the source line when available.

---

## 17. Determinism

v1 contains no randomness, concurrency, networking, or real-time input.

The same source program, initial world, and tick count must always produce the
same final simulation state.
