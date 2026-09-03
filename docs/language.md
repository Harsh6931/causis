# causis Language Specification v1

> This is the canonical v1 spec; see [PLAN.md](../PLAN.md) Section 0 for project scope.

causis v1 is a small domain-specific language for describing deterministic
2D grid simulations. A program declares one world, places simulation entities
on the grid, and attaches behavior to robots that runs on each simulation tick.

v1 is intentionally small. It is designed to make the first compiler pipeline
clear: source code -> tokens -> AST -> semantic checks -> simulation behavior.

---

## 1. Complete v1 Example

```causis
world 20 20;

robot R at 2 2;
target T at 17 17;

obstacle at 8 5;
obstacle at 8 6;
obstacle at 8 7;
obstacle at 8 8;

behavior R {
    every tick {
        if obstacle_ahead() {
            turn_right();
        }

        move_toward(T);
    }
}
```

---

## 2. Source Files

causis source files use the `.ls` extension.

A source file is a sequence of top-level declarations:

```causis
world 10 10;
robot R at 1 1;
target T at 8 8;

behavior R {
    every tick {
        move_right();
    }
}
```

Whitespace is not significant except where it separates tokens.

Semicolons terminate declarations and expression statements.

Blocks are delimited with `{` and `}`.

---

## 3. Comments

v1 supports line comments:

```causis
// This is ignored by the compiler.
robot R at 2 3;
```

Block comments are not part of v1.

---

## 4. Keywords

The following words are reserved in v1:

```text
world
robot
target
obstacle
behavior
every
tick
if
else
at
true
false
```

Reserved words cannot be used as identifiers.

---

## 5. Identifiers

Identifiers name robots, targets, and variables.

An identifier:

- starts with a letter or `_`
- may contain letters, digits, or `_`
- is case-sensitive

Valid identifiers:

```causis
R
robot1
target_A
speed
```

Invalid identifiers:

```causis
1robot
move-right
world
```

---

## 6. Literals and Values

v1 supports these literal types:

```causis
10
true
false
```

The core simulation features only require integers and booleans.

Numbers:

- integer literals are used for grid dimensions and coordinates
- floating-point literals are not accepted, but most v1, simulation operations reject them semantically

Strings:
Currently strings are removed completely as no use
- string literals are recognized for future diagnostics and metadata
- v1 simulation operations do not require strings

Booleans:

- `true` and `false` can be used in conditions

---

## 7. Coordinates and Grid Rules

Coordinates are written as:

```causis
x y
```

The origin is the top-left cell:

```text
(0, 0)
```

The positive x-axis points right.
The positive y-axis points down.

For a world declared as:

```causis
world 20 20;
```

valid x coordinates are `0` through `19`, and valid y coordinates are `0`
through `19`.

Coordinates outside the world are semantic errors.

---

## 8. World Declaration

Every v1 program must declare exactly one world.

```causis
world 20 20;
```

The first number is width.
The second number is height.

Rules:

- width and height must be positive integers
- the world must be declared before robots, targets, obstacles, or behaviors
- only one world declaration is allowed

---

## 9. Entities

v1 supports three entity kinds:

```text
robot
target
obstacle
```

### Robots

Robots are movable agents.

```causis
robot R at 2 2;
```

Rules:

- robot names must be unique
- a robot position must be inside the world
- a robot cannot start on an occupied cell

### Targets

Targets are named destination cells.

```causis
target T at 17 17;
```

Rules:

- target names must be unique
- a target position must be inside the world
- a target cannot start on an occupied cell

### Obstacles

Obstacles are static blocked cells.

```causis
obstacle at 8 5;
```

Rules:

- obstacles are unnamed in v1
- an obstacle position must be inside the world
- an obstacle cannot be placed on an occupied cell

---

## 10. Behaviors

A behavior block attaches logic to one robot.

```causis
behavior R {
    every tick {
        move_right();
    }
}
```

Rules:

- the behavior name must refer to a declared robot
- each robot may have at most one behavior block
- behavior blocks may only contain event blocks
- v1 supports only `every tick`

---

## 11. Tick Blocks

An `every tick` block runs once per simulation tick for its robot.

```causis
behavior R {
    every tick {
        move_toward(T);
    }
}
```

The compiler treats statements inside this block as the robot's repeated
runtime behavior.

---

## 12. Statements

v1 supports these executable statements inside `every tick` blocks:

```text
expression statement
if statement
block statement
variable assignment
```

### Expression Statement

Most simulation actions are function calls followed by `;`.

```causis
move_right();
move_toward(T);
```

### If Statement

```causis
if obstacle_ahead() {
    turn_right();
}
```

`else` is supported:

```causis
if obstacle_ahead() {
    turn_right();
} else {
    move_forward();
}
```

The condition must produce a boolean value.

### Block Statement

```causis
{
    move_right();
    move_down();
}
```

### Variable Assignment

Variables are created by assignment.

```causis
speed = 2;
```

Rules:

- variable names follow identifier rules
- variables are scoped to the current behavior
- v1 variables may hold integers or booleans
- variables are not required for the first simulation examples

---

## 13. Expressions

v1 expressions include:

```text
literals
identifiers
function calls
unary expressions
binary expressions
assignments
grouping with parentheses
```

Examples:

```causis
2 + 3
distance_to(T) < 5
obstacle_ahead()
speed = 2
(1 + 2) * 3
```

### Operators

v1 supports:

```text
=
+  -
*  /
== !=
< <= > >=
! 
```

Operator precedence, from highest to lowest:

```text
function call
grouping
unary ! -
* /
+ -
< <= > >=
== !=
assignment =
```

Assignment is right-associative.
Other binary operators are left-associative.

Arithmetic uses integers only. The `/` operator performs integer division
(truncates toward zero).

---

## 14. Built-in Simulation Operations

Built-in operations are called like functions.

### Actions

Actions change robot state or simulation state.

```causis
move_up();
move_down();
move_left();
move_right();
move_forward();
move_toward(T);
turn_left();
turn_right();
stop();
```

Rules:

- action calls are valid only inside a robot behavior
- `move_toward` takes exactly one target name
- movement into a wall, obstacle, or occupied cell does not move the robot
- failed movement records a collision for that robot

### Queries

Queries return values.

```causis
distance_to(T)
obstacle_ahead()
collision()
```

Rules:

- `distance_to` takes exactly one target name and returns an integer
- `obstacle_ahead` takes no arguments and returns a boolean
- `collision` takes no arguments and returns a boolean

---

## 15. Direction

Each robot has a direction.

The default starting direction in v1 is `right`.

Direction affects:

```causis
move_forward();
turn_left();
turn_right();
obstacle_ahead();
```

Absolute movement operations such as `move_up()` and `move_right()` do not
depend on the robot's current direction.

---

## 16. Runtime Model

The simulation advances in ticks.

At each tick:

1. each robot with a behavior executes its `every tick` block
2. robot behavior execution order is declaration order
3. movement updates the world immediately
4. collisions are recorded per robot
5. the tick counter increments after all robot behaviors run

This ordering keeps execution deterministic.

---

## 17. Semantic Errors

The compiler must reject programs that are syntactically valid but meaningless.

Examples:

```causis
move_toward(UnknownTarget);
```

```text
Semantic Error
Unknown target 'UnknownTarget'
```

v1 semantic errors include:

- missing world declaration
- duplicate world declaration
- invalid world dimensions
- entity declared before world
- duplicate robot name
- duplicate target name
- coordinates outside the world
- two entities placed on the same cell
- behavior for an unknown robot
- duplicate behavior for the same robot
- unknown function
- wrong function argument count
- wrong argument kind
- non-boolean `if` condition
- use of an unknown variable

---

## 18. Minimal Valid Programs

Smallest useful program:

```causis
world 5 5;

robot R at 0 0;

behavior R {
    every tick {
        move_right();
    }
}
```

World with static objects:

```causis
world 10 10;

robot R at 1 1;
target T at 8 8;
obstacle at 4 4;
```

Target navigation:

```causis
world 10 10;

robot R at 1 1;
target T at 8 8;

behavior R {
    every tick {
        move_toward(T);
    }
}
```

---

## 19. Out of Scope for v1

The following are not part of v1:

- multiple worlds
- user-defined functions
- user-defined agent types
- classes or objects
- arrays
- maps
- pathfinding syntax
- imports
- modules
- concurrency
- random behavior
- block comments
- floating-point literals
- string literals
- 3D simulation
- physics
- networking
- direct LLM control of the simulation

These features may be considered after the compiler, VM, and basic visualizer
are working.

---

## 20. v1 Keyword and Built-in Summary

Keywords:

```text
world robot target obstacle behavior every tick if else at true false
```

Built-in actions:

```text
move_up move_down move_left move_right move_forward move_toward
turn_left turn_right stop
```

Built-in queries:

```text
distance_to obstacle_ahead collision
```

Punctuation:

```text
( ) { } ; ,
```

Operators:

```text
= + - * / == != < <= > >= !
```
