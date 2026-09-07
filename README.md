# causis

A C++20-based domain-specific language, compiler, bytecode virtual machine, and simulation runtime for deterministic 2D grid simulations.

## Overview

causis is a C++20-based domain-specific language (DSL) and compiler for describing and executing deterministic 2D grid simulations. It provides simulation-oriented constructs such as worlds, robots, targets, obstacles, behaviors, and simulation ticks, allowing users to express grid-based behavior without implementing the underlying simulation engine themselves.

Unlike a general-purpose programming language, causis is designed around a specific domain: 2D grid simulation. Its language constructs directly represent concepts within that domain, while the compiler translates these high-level descriptions into an executable form.

A causis program passes through a complete compiler pipeline. Source code is transformed from tokens into an abstract syntax tree (AST), checked for semantic correctness, lowered into an intermediate representation (IR), optimized, and compiled into bytecode. A custom virtual machine (VM) executes the bytecode and drives the simulation runtime.

The project brings together compiler construction and simulation runtime design in a single system, demonstrating how a purpose-built language can be transformed through multiple compiler stages and ultimately used to control a deterministic simulation. The resulting simulation state can be recorded as a tick log and explored through a static 2D visualization.

```
                                           causis Source

                                                ↓

                                              Lexer

                                                ↓

                                              Parser

                                                ↓

                                               AST

                                                ↓

                                         Semantic Analysis

                                                ↓

                                               IR

                                                ↓

                                           Optimization

                                                ↓

                                             Bytecode

                                                ↓

                                        Virtual Machine(VM)

                                                ↓

                                        Simulation Runtime

                                                ↓

                                    Tick Log / 2D Visualization
```

## Why causis?

Describing grid-based simulations in a general-purpose language often involves implementation details such as grid management, entity handling, collision checking, and simulation updates.

**causis** abstracts these details into domain-specific constructs such as worlds, robots, targets, obstacles, and behaviors, allowing users to describe **what the simulation should do** rather than how the underlying engine works.

At the same time, causis provides a practical domain for exploring the complete compiler pipeline—from source code to bytecode and virtual-machine execution.

## Key features

- **Domain-Specific Language** — Provides constructs for defining 2D grid simulations.
- **Compiler Pipeline** — Transforms source code through lexing, parsing, semantic analysis, IR generation, optimization, and bytecode generation.
- **Semantic Analysis** — Validates declarations, identifiers, types, coordinates, and language rules.
- **IR Optimization** — Applies constant folding and dead-code elimination.
- **Bytecode Virtual Machine** — Executes compiled bytecode using a custom stack-based VM.
- **Simulation Runtime** — Executes deterministic grid simulations with movement, directions, targets, obstacles, collisions, and ticks.
- **Built-in Operations** — Provides movement, turning, distance, obstacle detection, and collision operations.
- **Command-Line Interface** — Provides commands for inspecting, compiling, and executing causis programs.
- **2D Visualization** — Displays recorded simulation states using HTML and Canvas.
- **Automated Testing** — Validates compiler and runtime components using GoogleTest and CTest.

## Architecture

A causis program is processed end to end through the compiler pipeline, executed by the VM against the simulation runtime, and optionally recorded for visualization.

```
                              .ls Source File

                                    ↓

                                  Lexer

                                    ↓

                                 Tokens

                                    ↓

                                 Parser

                                    ↓

                            Abstract Syntax Tree

                                    ↓

                          Semantic Analysis
                         (symbol table, checks)

                                    ↓

                            IR Generation
                               (lowering)

                                    ↓

                               Optimizer
                    (constant folding, dead-code elimination)

                                    ↓

                          Bytecode Compiler

                                    ↓

                             Bytecode Module

                                    ↓

                        Stack-based Virtual Machine

                                    ↓

                          Simulation Runtime
                    (world, robots, targets, obstacles)

                                    ↓

                              Tick Log (JSON)

                                    ↓

                    Static Visualizer (HTML / Canvas)
```

## Language overview

causis v1 is a small DSL for deterministic 2D grid simulations. Full syntax and semantics are defined in the project documentation:

- [Language specification](docs/language.md) — keywords, declarations, expressions, and built-in operations
- [Grammar](docs/grammar.md) — formal grammar for the parser
- [Semantics](docs/semantics.md) — runtime behavior, movement rules, and simulation model

Major constructs at a glance:

| Construct | Example |
|-----------|---------|
| **World** | `world 20 20;` |
| **Robot** | `robot R at 2 2;` |
| **Obstacle** | `obstacle at 8 5;` |
| **Behavior** | `behavior R { every tick { move_forward(); } }` |

Source files use the `.ls` extension. See [examples/](examples/) for complete programs.

## Complete example program

The canonical Definition-of-Done example declares a 20×20 world, places a robot and target, adds a vertical obstacle wall, and attaches navigation behavior to the robot:

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

**What it does:** The program sets up a grid world with robot `R` at `(2, 2)` and target `T` at `(17, 17)`, with a column of obstacles blocking the direct path at `x = 8`. On every simulation tick, robot `R` checks whether an obstacle lies directly ahead in its current facing direction; if so, it turns right. It then attempts to move one step closer to `T` using Manhattan-distance movement. The simulation runtime applies each movement immediately and records collisions when a move is blocked.

See [examples/path_to_target.ls](examples/path_to_target.ls) for an extended version with goal detection and richer navigation.

## Compilation pipeline

Each stage transforms the program one step closer to executable simulation behavior:

| Stage | Input | Output | Role |
|-------|-------|--------|------|
| **Lexer** | Source text (`.ls`) | Token stream | Breaks source into keywords, identifiers, literals, and punctuation. |
| **Parser** | Tokens | AST | Builds a tree of declarations and statements from the token stream. |
| **Semantic analysis** | AST | Validated AST | Checks language rules: one world, unique names, in-bounds coordinates, declared-before-use, and type-correct expressions. |
| **IR generation** | AST | IR module | Lowers high-level constructs (behaviors, control flow, built-in calls) into a simpler intermediate representation. |
| **Optimizer** | IR module | IR module | Applies constant folding and dead-code elimination to remove redundant work. |
| **Bytecode compiler** | IR module | Bytecode module | Emits stack-machine instructions and operand data for the VM. |
| **Virtual machine** | Bytecode module | Runtime effects | Executes instructions each tick, calling into the simulation runtime for movement and queries. |

The CLI exposes individual stages for inspection (`tokenize`, `parse`, `semantic`, `ir`, `optimize`, `disassemble`) and runs the full pipeline with `run`.

## Simulation model

The simulation runtime is separate from the compiler. Once a program is loaded, the **world** holds all simulation state; the VM only invokes runtime operations (move, turn, query) against that world.

### World and entities

- **World** — A fixed-size 2D grid. Coordinates start at `(0, 0)` in the top-left; `x` increases right and `y` increases down.
- **Robot** — A named agent with position, facing direction (default `right`), and a per-tick collision flag.
- **Target** — A named, stationary goal cell. Robots can query distance and move toward targets; target cells are walkable.
- **Obstacle** — A static blocked cell. Robots cannot enter obstacle cells.

Each grid cell holds at most one occupying entity at declaration time. During simulation, robots move across the grid according to behavior rules.

### Tick simulation

Time advances in discrete integer **ticks**. One tick follows this order:

```
  Start tick N
       ↓
  Clear collision flags on all robots
       ↓
  For each robot (in declaration order):
    run its `every tick` behavior block
    (movements apply immediately as statements execute)
       ↓
  Increment global tick counter → tick N + 1
```

This fixed ordering makes execution **deterministic**: the same program and tick limit always produce the same sequence of world states. After each tick (or on demand), the runtime can snapshot the world into a **tick log** for the static HTML visualizer.

For full movement, collision, and built-in operation rules, see [docs/semantics.md](docs/semantics.md).

