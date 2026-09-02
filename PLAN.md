First, spend 2–4 days designing the actual causis language. The most important decision is what the 15–25 keywords/operators of the language will be and what a genuinely good simulation program looks like.

Once that syntax is right, the rest of the compiler becomes much easier to structure.
And because you're starting from scratch, I'd make the first target extremely small:


# causis — Implementation Plan

> **causis is a domain-specific language (DSL) and compiler for describing, controlling, and visualizing 2D grid-based simulations.**
>
> The project combines compiler construction, a bytecode virtual machine, simulation execution, visualization, and an optional LLM-powered natural-language frontend.

---

# 0. Scope Revision (Current)

This plan was written as a full 7-milestone vision. After factoring in real
constraints — this runs parallel to active placement prep (OA rounds, DSA
revision) and is not the primary resume project — the working scope has
been trimmed. The rest of this document is kept as the original reference
plan; sections affected by the cuts below are marked inline as **[CUT]**
or **[SIMPLIFIED]**.

## Cut entirely (for now)

- Stage 16 — LLM Integration
- Stage 17 — LLM Validation Loop
- Stage 18 — LLM Evaluation
- Milestone 6 (Intelligent Frontend) and Milestone 7 (Final System)

These are not deleted from the doc as reference for "if there's time later,"
but they are **not** part of the working plan or the Definition of Done.

## Simplified

- **Optimizer (Stage 8):** keep only constant folding and dead-code
  elimination. Skip unreachable-behavior elimination, redundant-operation
  elimination, and static bounds checking unless time allows.
- **Visualization (Stage 12):** a static HTML/Canvas page that reads a
  tick-log (JSON) produced by the VM and steps through it. No live
  run/pause/speed controls, no styling polish. Stage 13 (Compiler
  Visualization / multi-pane pipeline view) is downgraded from
  "centerpiece" to optional stretch goal.

## New real Definition of Done

```
Source → Lexer → Parser → AST → Semantic Analysis →
Simulation Model → IR → (minimal) Bytecode → VM → Basic 2D Visualization
```

Milestone 4 (VM working, compiled programs execute) is the real checkpoint
— a legitimate, explainable project on its own if placement season forces
a stop there. Everything past it (visualization, pathfinding) is a bonus,
not a dependency for the project being "done enough to talk about."

---

# 1. Project Vision

## 1.1 Problem

Traditional programming languages are unnecessarily expressive for simple simulation tasks.

For example, describing:

> "Create a 20×20 world with a robot at (2,2), a target at (18,18), and obstacles between them. Make the robot move toward the target while avoiding obstacles."

would require writing general-purpose application code.

causis provides a constrained domain-specific language where simulation concepts are first-class constructs.

The system should allow a user to:

1. Describe a simulation world.
2. Define agents and objects.
3. Define agent behavior.
4. Compile the program.
5. Execute it using a virtual machine.
6. Visualize the resulting simulation.
7. Optionally describe the simulation in natural language and use an LLM to generate causis code.

---

# 2. Core Project Goals

The final system should demonstrate:

- Lexical analysis
- Parsing
- Abstract Syntax Trees
- Semantic analysis
- Symbol tables
- Intermediate representation
- Bytecode generation
- Compiler optimization
- Virtual machine execution
- Event-driven simulation
- 2D visualization
- Runtime error handling
- Optional LLM-assisted code generation

The project should **not** attempt to become a general-purpose language.

---

# 3. Project Architecture

The complete system:

```text
                    ┌─────────────────────┐
                    │       User          │
                    └──────────┬──────────┘
                               │
                    Natural Language (optional)
                               │
                               ▼
                    ┌─────────────────────┐
                    │        LLM          │
                    │  NL → causis DSL │
                    └──────────┬──────────┘
                               │
                         causis Code
                               │
                               ▼
┌───────────────────────────────────────────────────────────┐
│                    causis COMPILER                      │
│                                                           │
│  Source → Lexer → Parser → AST → Semantic Analysis       │
│                            ↓                              │
│                           IR                              │
│                            ↓                              │
│                       Optimizer                           │
│                            ↓                              │
│                       Bytecode                            │
└───────────────────────────┬───────────────────────────────┘
                            │
                            ▼
                    ┌─────────────────┐
                    │   causis VM   │
                    └────────┬────────┘
                             │
                             ▼
                  ┌─────────────────────┐
                  │ Simulation Runtime  │
                  │                     │
                  │ World               │
                  │ Grid                │
                  │ Agents              │
                  │ Objects             │
                  │ Events              │
                  │ Simulation Clock    │
                  └──────────┬──────────┘
                             │
                             ▼
                    ┌─────────────────┐
                    │  2D Visualizer  │
                    └─────────────────┘
```

---

# 4. Scope Philosophy

The project follows three principles.

## Principle 1 — Small language

Only add syntax that has a direct simulation purpose.

## Principle 2 — Compiler first

The LLM and visualization are supporting components.

The compiler must work independently.

## Principle 3 — Simulation over game engine

The project is a simulation system, not a game engine.

Do not add:

- 3D graphics
- realistic physics
- audio
- networking
- multiplayer
- complex animation
- asset pipelines

---

# 5. Target Language

The initial language should support:

- world declaration
- grid dimensions
- agents
- static obstacles
- targets
- positions
- variables
- arithmetic
- conditions
- movement
- events
- simulation ticks
- functions
- basic agent behavior

Example:

```text
world 20 20;

robot R at 2 2;
target T at 17 17;

obstacle at 5 5;
obstacle at 5 6;
obstacle at 5 7;

behavior R {
    every tick {
        move_toward(T);
    }
}
```

The exact syntax can evolve during Stage 1.

---

# 6. Stage 0 — Project Setup

## Goal

Create the C++ project structure and development environment.

## Tasks

- [ ] Create Git repository
- [ ] Configure CMake
- [ ] Configure C++ standard
- [ ] Create `src/`
- [ ] Create `include/`
- [ ] Create `tests/`
- [ ] Create `examples/`
- [ ] Create `docs/`
- [ ] Create `frontend/`
- [ ] Create `runtime/`
- [ ] Create `tools/`
- [ ] Configure compiler warnings
- [ ] Add unit testing framework
- [ ] Create initial executable
- [ ] Create `.gitignore`
- [ ] Create initial README
- [ ] Create PLAN.md

Recommended structure:

```text
causis/
│
├── CMakeLists.txt
├── README.md
├── PLAN.md
│
├── src/
│   ├── lexer/
│   ├── parser/
│   ├── ast/
│   ├── semantic/
│   ├── ir/
│   ├── optimizer/
│   ├── bytecode/
│   ├── vm/
│   ├── runtime/
│   └── main.cpp
│
├── include/
│
├── tests/
│
├── examples/
│
├── docs/
│
├── frontend/
│
└── tools/
```

### Checkpoint

```text
causis --help
```

works.

---

# 7. Stage 1 — Language Specification

## Goal

Define the language before implementing the compiler.

Write:

```text
docs/language.md
docs/grammar.md
docs/semantics.md
```

## Define

### Literals

```text
10
3.14
true
false
"hello"
```

### Identifiers

```text
robot1
target
speed
```

### World

```text
world 20 20;
```

### Positions

```text
robot R at 2 3;
```

### Objects

```text
obstacle at 5 5;
target T at 18 18;
```

### Behaviors

```text
behavior R {
    every tick {
        move_right();
    }
}
```

### Conditions

```text
if obstacle_ahead {
    turn_right();
}
```

### Events

```text
when collision {
    turn_right();
}
```

### Variables

```text
speed = 2;
```

---

# 8. Stage 2 — Lexer

## Goal

Convert source code into tokens.

Example:

```text
robot R at 2 3;
```

becomes:

```text
ROBOT
IDENTIFIER(R)
AT
NUMBER(2)
NUMBER(3)
SEMICOLON
```

## Tasks

- [ ] Define `TokenType`
- [ ] Implement source reader
- [ ] Implement whitespace handling
- [ ] Implement comments
- [ ] Implement identifiers
- [ ] Implement keywords
- [ ] Implement integer literals
- [ ] Implement floating-point literals
- [ ] Implement strings
- [ ] Implement operators
- [ ] Implement punctuation
- [ ] Track line numbers
- [ ] Track columns
- [ ] Implement lexer errors
- [ ] Implement token dump CLI

Example:

```text
causis tokenize examples/robot.ls
```

### Checkpoint

Input source produces a correct token stream.

---

# 9. Stage 3 — Parser

## Goal

Convert tokens into an AST.

Use recursive descent for statements and Pratt parsing where expression precedence is required.

## AST categories

### Expressions

```text
LiteralExpr
VariableExpr
BinaryExpr
UnaryExpr
CallExpr
AssignmentExpr
```

### Statements

```text
WorldStmt
RobotStmt
TargetStmt
ObstacleStmt
BehaviorStmt
EveryTickStmt
WhenStmt
IfStmt
BlockStmt
VarStmt
ExpressionStmt
```

## Tasks

- [ ] Design AST base classes
- [ ] Implement expression parser
- [ ] Implement statement parser
- [ ] Implement blocks
- [ ] Implement behavior blocks
- [ ] Implement event blocks
- [ ] Implement error recovery
- [ ] Implement AST printer

Example:

```text
causis parse examples/robot.ls
```

Output:

```text
World(20,20)
 ├── Robot(R,2,2)
 ├── Target(T,17,17)
 └── Behavior(R)
      └── EveryTick
           └── MoveToward(T)
```

### Checkpoint

Valid programs produce correct ASTs.

Invalid programs produce useful errors.

---

# 10. Stage 4 — Semantic Analysis

## Goal

Ensure the program is meaningful, not merely syntactically valid.

This is an important compiler stage and should not be skipped.

## Symbol Table

Track:

```text
Worlds
Agents
Targets
Objects
Variables
Functions
```

## Semantic checks

Examples:

```text
✓ Robot exists
✓ Target exists
✓ Coordinates are within world
✓ Variable exists
✓ Function exists
✓ Function arguments are correct
✓ Movement operation is valid
✓ Event is valid
```

Invalid example:

```text
move_toward(UnknownRobot);
```

Compiler:

```text
Semantic Error:
Unknown simulation object 'UnknownRobot'

line 7:
    move_toward(UnknownRobot);
                ^^^^^^^^^^^^
```

## Tasks

- [ ] Build symbol table
- [ ] Implement scope handling
- [ ] Validate world
- [ ] Validate coordinates
- [ ] Validate object references
- [ ] Validate behavior references
- [ ] Validate variables
- [ ] Validate functions
- [ ] Validate built-in simulation operations
- [ ] Implement semantic diagnostics

### Checkpoint

Semantically invalid programs fail before execution.

---

# 11. Stage 5 — Simulation Model

## Goal

Build the independent 2D simulation engine.

This stage should work **without the compiler**.

## World

```cpp
class World {
    int width;
    int height;
    Grid grid;
};
```

## Grid

Each cell can contain:

```text
EMPTY
OBSTACLE
TARGET
AGENT
```

## Position

```cpp
struct Position {
    int x;
    int y;
};
```

## Agent

```cpp
class Agent {
    string name;
    Position position;
    Direction direction;
};
```

## Simulation

```cpp
class Simulation {
    World world;
    vector<Agent> agents;
    vector<Event> events;
    int tick;
};
```

## Tasks

- [ ] Implement grid
- [ ] Implement positions
- [ ] Implement agents
- [ ] Implement obstacles
- [ ] Implement targets
- [ ] Implement collision checking
- [ ] Implement movement
- [ ] Implement simulation tick
- [ ] Implement reset
- [ ] Implement deterministic execution
- [ ] Write simulation unit tests

### Checkpoint

A C++ test can manually create a world and move an agent.

---

# 12. Stage 6 — Simulation Semantics

## Goal

Define exactly what causis operations mean at runtime.

Example:

```text
move_right();
```

means:

```text
new_position = current_position + (1,0)

if new_position is valid
    move
else
    collision
```

## Initial operations

```text
move_up()
move_down()
move_left()
move_right()

turn_left()
turn_right()

move_forward()

move_toward(target)

stop()

distance_to(target)

obstacle_ahead()

collision()
```

## Events

```text
every tick
when collision
when reaches target
```

## Tasks

- [ ] Define operation semantics
- [ ] Define event semantics
- [ ] Define tick ordering
- [ ] Define collision behavior
- [ ] Define multi-agent behavior
- [ ] Document deterministic execution rules

### Checkpoint

The simulation behavior is deterministic and documented.

---

# 13. Stage 7 — Intermediate Representation

## Goal

Introduce an IR between the AST and bytecode.

This is important for demonstrating genuine compiler architecture.

Example source:

```text
behavior R {
    every tick {
        move_right();
    }
}
```

IR:

```text
CREATE_AGENT R

LOOP:
    LOAD_AGENT R
    MOVE_RIGHT
    TICK
    JUMP LOOP
```

## Tasks

- [ ] Design IR instruction set
- [ ] Define IR values
- [ ] Define labels
- [ ] Define basic blocks
- [ ] Implement IR builder
- [ ] Implement AST → IR translation
- [ ] Implement IR printer

Example:

```text
causis compile --ir robot.ls
```

### Checkpoint

A causis source program produces readable IR.

---

# 14. Stage 8 — Compiler Optimizations **[SIMPLIFIED — see Section 0]**

## Goal

Implement compiler optimizations relevant to the simulation DSL.

Start simple. **Working scope: Optimization 1 and 2 only.** Optimizations
3-5 below are stretch goals, not required for Definition of Done.

## Optimization 1 — Constant folding

```text
speed = 2 + 3;
```

becomes:

```text
speed = 5;
```

## Optimization 2 — Dead code elimination

```text
if false {
    move_right();
}
```

can be eliminated.

## Optimization 3 — Unreachable behavior

Remove statements after unconditional termination.

## Optimization 4 — Redundant operation elimination

Example:

```text
turn_left();
turn_right();
```

can potentially become:

```text
NO_OP
```

depending on semantics.

## Optimization 5 — Static bounds checking

If the compiler can prove that an operation goes outside the grid, report it at compile time.

## Tasks

- [ ] Build optimizer pass framework
- [ ] Implement constant folding
- [ ] Implement dead-code elimination
- [ ] Add optimization tests
- [ ] Add before/after IR output

### Stretch (not required)

- [ ] Implement unreachable-code elimination
- [ ] Implement at least one simulation-specific optimization

### Checkpoint

```text
Source
 ↓
IR
 ↓
Optimizer
 ↓
Optimized IR
```

is demonstrable.

---

# 15. Stage 9 — Bytecode Compiler

## Goal

Compile optimized IR into compact bytecode.

Possible instructions:

```text
HALT

PUSH_CONST
LOAD
STORE

ADD
SUB
MUL
DIV

EQ
LT
GT
NOT

JUMP
JUMP_IF_FALSE

CREATE_AGENT
CREATE_TARGET
CREATE_OBSTACLE

MOVE_UP
MOVE_DOWN
MOVE_LEFT
MOVE_RIGHT

TURN_LEFT
TURN_RIGHT

MOVE_TOWARD

CHECK_COLLISION
CHECK_OBSTACLE

TICK

CALL
RETURN

PRINT
```

## Tasks

- [ ] Define opcode enum
- [ ] Implement bytecode container
- [ ] Implement constant pool
- [ ] Implement label resolution
- [ ] Implement IR → bytecode compiler
- [ ] Add source line information
- [ ] Implement disassembler

Example:

```text
causis disassemble robot.ls
```

Output:

```text
0000 CREATE_WORLD 20 20
0003 CREATE_AGENT 0 2 2
0007 LOAD_AGENT 0
0009 MOVE_RIGHT
0010 TICK
0011 JUMP 0007
```

### Checkpoint

Every supported language construct compiles to bytecode.

---

# 16. Stage 10 — causis Virtual Machine

## Goal

Execute bytecode.

## VM components

```text
Value Stack
Instruction Pointer
Call Stack
Local Variables
Simulation Reference
```

## Tasks

- [ ] Implement VM class
- [ ] Implement instruction dispatch
- [ ] Implement stack
- [ ] Implement variables
- [ ] Implement arithmetic
- [ ] Implement branching
- [ ] Implement simulation instructions
- [ ] Implement event handling
- [ ] Implement tick execution
- [ ] Implement runtime errors
- [ ] Implement stack traces
- [ ] Add VM tests

### Checkpoint

```text
.ls
 ↓
bytecode
 ↓
VM
 ↓
simulation state
```

works correctly.

---

# 17. Stage 11 — Compiler CLI

## Goal

Provide a clean command-line interface.

Commands:

```text
causis tokenize program.ls

causis parse program.ls

causis semantic program.ls

causis ir program.ls

causis optimize program.ls

causis disassemble program.ls

causis run program.ls
```

Optional:

```text
causis compile program.ls
```

## Checkpoint

A stranger should be able to clone the repository and run:

```text
causis run examples/robot.ls
```

without understanding the implementation.

---

# 18. Stage 12 — 2D Visualizer **[SIMPLIFIED — see Section 0]**

## Goal

Make simulation execution visually observable.

**Working scope:** the VM writes a tick-by-tick log to JSON (world state,
agent positions, obstacles, targets per tick). A static HTML/Canvas page
loads that JSON and lets you step through it with a single "Next tick"
button. No live connection to a running VM, no Run/Pause/speed controls,
no animation — the sections below (Required controls, Step execution)
describe the original fuller vision; treat them as optional upgrades, not
requirements.

The visualizer should display:

```text
┌─────────────────────────────┐
│                             │
│  🤖                         │
│                             │
│       ███                   │
│       █                     │
│       █          🎯         │
│                             │
│                             │
└─────────────────────────────┘
```

## Required controls

```text
▶ Run
⏸ Pause
⏭ Step
↻ Reset
1×
2×
5×
```

## Display

Show:

- grid
- agents
- obstacles
- targets
- current tick
- agent coordinates
- current state

## Step execution

When the user presses:

```text
Step
```

the VM executes one simulation step.

Example:

```text
Tick 10

Instruction:
MOVE_TOWARD

Agent R:
(7,8) → (8,8)
```

The visualizer updates immediately.

## Checkpoint

A simulation can be run visually from start to finish.

---

# 19. Stage 13 — Compiler Visualization **[OPTIONAL STRETCH — see Section 0]**

## Goal

Connect the compiler internals to the visual interface. Not required for
Definition of Done — only attempt this if Stages 0-12 are done with time
to spare.

Create views for:

### Source

```text
world 20 20;
robot R at 2 2;
```

### Tokens

```text
WORLD
NUMBER
NUMBER
ROBOT
IDENTIFIER
...
```

### AST

```text
World
 ├── Robot
 └── Target
```

### IR

```text
CREATE_WORLD
CREATE_AGENT
MOVE_TOWARD
TICK
```

### Bytecode

```text
0000 CREATE_WORLD
0003 CREATE_AGENT
0007 MOVE_TOWARD
0010 TICK
```

### VM

```text
IP: 0010
Tick: 12

Agent R
Position: (9,10)
```

## Checkpoint

The user can visually follow:

```text
Source
 ↓
Tokens
 ↓
AST
 ↓
IR
 ↓
Bytecode
 ↓
VM
 ↓
Simulation
```

(Originally envisioned as the centerpiece of the final demonstration —
downgraded to optional per Section 0.)

---

# 20. Stage 14 — Simulation Scenarios

## Goal

Create reusable demonstrations.

### Scenario 1 — Basic movement

One agent moves across an empty grid.

### Scenario 2 — Obstacle collision

Agent encounters an obstacle.

### Scenario 3 — Target navigation

Agent moves toward a target.

### Scenario 4 — Maze

Agent navigates a small maze.

### Scenario 5 — Multiple agents

Multiple agents operate simultaneously.

### Scenario 6 — Event-driven behavior

Agent reacts to collision.

### Scenario 7 — Pathfinding

Optional advanced scenario.

---

# 21. Stage 15 — Pathfinding

## Goal

Add one meaningful algorithmic component.

Implement:

```text
BFS
```

first.

Then optionally:

```text
A*
```

Example:

```text
move_to(T);
```

can generate a path:

```text
🤖 → → ↓ ↓ → → ↓ → 🎯
```

## Tasks

- [ ] Implement BFS
- [ ] Represent paths
- [ ] Add obstacle awareness
- [ ] Integrate with simulation
- [ ] Expose pathfinding as a DSL operation
- [ ] Visualize selected path

### Optional

Implement A* and compare:

```text
BFS vs A*
```

in terms of explored cells and execution time.

---

# 22. Stage 16 — LLM Integration **[CUT — see Section 0]**

> Not part of the working plan. Kept as reference for if there's time
> after the core compiler + simulation + VM + basic visualization is done.

## Goal

Allow users to describe simulations using natural language.

Example:

> Create a 20×20 world. Put a robot at the top-left and a target at the bottom-right. Add a wall in the middle. Make the robot reach the target while avoiding the wall.

LLM produces:

```text
world 20 20;

robot R at 1 1;
target T at 18 18;

obstacle at 10 5;
obstacle at 10 6;
obstacle at 10 7;

behavior R {
    every tick {
        move_toward(T);
        avoid_obstacles();
    }
}
```

Then:

```text
LLM
 ↓
causis source
 ↓
Compiler
 ↓
Simulation
```

## Important constraint

The LLM must **not directly control the simulation**.

The compiler remains the authority.

The LLM only generates causis source code.

---

# 23. Stage 17 — LLM Validation Loop **[CUT — see Section 0]**

## Goal

Use compiler diagnostics to correct generated programs.

Pipeline:

```text
User request
     ↓
LLM
     ↓
causis code
     ↓
Compiler
     ↓
 ┌───┴────┐
 │        │
Valid    Error
 │        │
 ↓        ↓
Run      Error
          ↓
         LLM
          ↓
       Correction
```

Example:

```text
Semantic Error:
Unknown operation 'move_randomly'
Did you mean 'move_random'?
```

The LLM receives the compiler diagnostic and produces corrected code.

## Tasks

- [ ] Design LLM prompt
- [ ] Require causis-only output
- [ ] Parse returned code
- [ ] Validate generated code
- [ ] Return compiler diagnostics
- [ ] Implement correction loop
- [ ] Limit correction attempts
- [ ] Prevent arbitrary code execution

---

# 24. Stage 18 — LLM Evaluation **[CUT — see Section 0]**

## Goal

Treat the LLM integration as an experiment rather than simply an API feature.

Create a benchmark set.

Example:

```text
Test 1:
Create a robot that moves right.

Test 2:
Create a robot that avoids obstacles.

Test 3:
Create three robots and one target.

Test 4:
Create a maze and navigate through it.
```

Measure:

```text
Syntax validity
Semantic validity
Successful execution
Correction rate
Number of attempts
```

Example report:

```text
Total prompts:             50
Valid first attempts:      43
Syntax failures:             4
Semantic failures:           3
Successful after correction:49
```

This provides an objective evaluation of the LLM frontend.

---

# 25. Stage 19 — Compiler Performance Evaluation

## Goal

Demonstrate why compilation and optimization matter.

Compare:

```text
Tree-walking interpreter
        vs
Bytecode VM
        vs
Optimized bytecode VM
```

Use simulations with:

```text
10 agents
50 agents
100 agents
500 agents
```

Measure:

```text
Execution time
Ticks/second
Instructions executed
Memory usage
```

Example:

| Agents | Interpreter | VM | Optimized VM |
|---:|---:|---:|---:|
| 10 | — | — | — |
| 50 | — | — | — |
| 100 | — | — | — |
| 500 | — | — | — |

Actual values should be generated experimentally.

---

# 26. Stage 20 — Testing

## Lexer tests

- [ ] Keywords
- [ ] Identifiers
- [ ] Numbers
- [ ] Strings
- [ ] Operators
- [ ] Comments
- [ ] Invalid characters

## Parser tests

- [ ] Valid world
- [ ] Valid agents
- [ ] Behaviors
- [ ] Events
- [ ] Conditions
- [ ] Invalid syntax

## Semantic tests

- [ ] Unknown agent
- [ ] Unknown target
- [ ] Invalid coordinates
- [ ] Invalid function
- [ ] Invalid argument count
- [ ] Invalid operation

## Runtime tests

- [ ] Movement
- [ ] Collision
- [ ] Target detection
- [ ] Events
- [ ] Multiple agents
- [ ] Reset
- [ ] Determinism

## VM tests

- [ ] Arithmetic
- [ ] Branching
- [ ] Calls
- [ ] Variables
- [ ] Simulation instructions
- [ ] Runtime errors

## Integration tests

Every example program should be tested through:

```text
Source
 → Lexer
 → Parser
 → Semantic Analysis
 → IR
 → Optimization
 → Bytecode
 → VM
 → Simulation
```

---

# 27. Stage 21 — Error Reporting

## Goal

Make compiler errors professional.

Bad:

```text
Error.
```

Good:

```text
Semantic Error

Unknown target 'T2'

  14 | move_toward(T2);
                  ^^

Target 'T2' was not declared.
```

Implement:

- [ ] Error category
- [ ] Line number
- [ ] Column number
- [ ] Source excerpt
- [ ] Caret indicator
- [ ] Helpful explanation
- [ ] Suggested correction where possible

---

# 28. Stage 22 — Project Demonstration

The final demonstration should follow one complete scenario.

## Demo sequence

### Step 1 — Natural language

```text
Create a robot that reaches a target
while avoiding obstacles.
```

### Step 2 — LLM

Generate causis.

### Step 3 — Source

Show generated DSL.

### Step 4 — Compiler

Show:

```text
Tokens
 ↓
AST
 ↓
Semantic Analysis
 ↓
IR
 ↓
Optimized IR
 ↓
Bytecode
```

### Step 5 — VM

Show:

```text
Instruction Pointer
Stack
Current Tick
Agent State
```

### Step 6 — Simulation

Robot moves through the grid.

### Step 7 — Step mode

Pause the simulation.

Execute instructions one at a time.

### Step 8 — Error demonstration

Give the LLM an intentionally invalid request/output.

Show:

```text
LLM output
 ↓
Compiler
 ↓
Semantic error
 ↓
Correction
 ↓
Valid program
```

### Step 9 — Optimization/performance

Show:

```text
Before optimization
vs
After optimization
```

---

# 29. Final Feature Set

The final target should contain:

## Compiler

- [ ] Lexer
- [ ] Parser
- [ ] AST
- [ ] Semantic analyzer
- [ ] Symbol table
- [ ] IR
- [ ] Optimizer
- [ ] Bytecode compiler
- [ ] Disassembler
- [ ] Error reporting

## Runtime

- [ ] Virtual machine
- [ ] Stack
- [ ] Variables
- [ ] Simulation instructions
- [ ] Events
- [ ] Tick system

## Simulation

- [ ] 2D grid
- [ ] Agents
- [ ] Obstacles
- [ ] Targets
- [ ] Movement
- [ ] Collision
- [ ] Multiple agents
- [ ] Pathfinding

## Visualization

- [ ] Grid renderer
- [ ] Run
- [ ] Pause
- [ ] Step
- [ ] Reset
- [ ] Speed control
- [ ] Agent information
- [ ] Compiler pipeline visualization

## LLM **[CUT — see Section 0]**

- [ ] Natural-language input
- [ ] DSL generation
- [ ] Compiler validation
- [ ] Error feedback
- [ ] Automatic correction
- [ ] Evaluation benchmark

---

# 30. Features Explicitly Out of Scope

Do **not** add these unless the core project is completely finished:

- [ ] 3D simulation
- [ ] Realistic physics
- [ ] Multiplayer
- [ ] Networking
- [ ] Distributed simulation
- [ ] Full OOP
- [ ] Garbage collector
- [ ] Complex type system
- [ ] Native machine-code backend
- [ ] GPU acceleration
- [ ] Full natural-language compiler
- [ ] Autonomous LLM agents
- [ ] Natural-language / LLM frontend of any kind (Stages 16-18, Milestones
      6-7) — cut for this project cycle, see Section 0
- [ ] Live run/pause/speed-control visualizer and multi-pane compiler
      pipeline view (Stage 13) — downgraded to optional stretch, see
      Section 0

The objective is a **focused compiler + simulation system**, not an entire programming ecosystem.

---

# 31. Recommended Technology Stack

## Compiler

```text
C++
C++20
CMake
```

## Testing

```text
GoogleTest
```

or a lightweight custom test framework.

## Visualization

Recommended:

```text
HTML
CSS
JavaScript
Canvas
```

This allows the compiler state and simulation to be shown in one interface.

Alternative:

```text
Raylib
```

if a native application is preferred.

## LLM

Use an external LLM API only after the compiler works independently.

The LLM layer should be isolated from the compiler core.

---

# 32. Suggested Repository Structure

```text
causis/
│
├── CMakeLists.txt
├── README.md
├── PLAN.md
├── LICENSE
│
├── include/
│   ├── lexer/
│   ├── parser/
│   ├── ast/
│   ├── semantic/
│   ├── ir/
│   ├── optimizer/
│   ├── bytecode/
│   ├── vm/
│   └── runtime/
│
├── src/
│   ├── lexer/
│   ├── parser/
│   ├── ast/
│   ├── semantic/
│   ├── ir/
│   ├── optimizer/
│   ├── bytecode/
│   ├── vm/
│   ├── runtime/
│   └── main.cpp
│
├── tests/
│   ├── lexer/
│   ├── parser/
│   ├── semantic/
│   ├── ir/
│   ├── optimizer/
│   ├── vm/
│   └── integration/
│
├── examples/
│   ├── basic_move.ls
│   ├── collision.ls
│   ├── target.ls
│   ├── maze.ls
│   └── multi_agent.ls
│
├── docs/
│   ├── grammar.md
│   ├── language.md
│   ├── semantics.md
│   ├── architecture.md
│   └── optimization.md
│
├── frontend/
│   ├── index.html
│   ├── style.css
│   └── app.js
│
└── tools/
    └── benchmark/
```

---

# 33. Development Order

Do **not** build everything simultaneously.

Follow this dependency chain:

```text
Language specification
        ↓
Lexer
        ↓
Parser
        ↓
AST
        ↓
Semantic analysis
        ↓
Simulation model
        ↓
Simulation semantics
        ↓
IR
        ↓
Optimizer
        ↓
Bytecode
        ↓
VM
        ↓
CLI
        ↓
Visualizer
        ↓
Pathfinding
        ↓
LLM
        ↓
LLM correction loop
        ↓
Benchmarking
        ↓
Polish
```

---

# 34. Milestones

## Milestone 1 — Language Frontend

```text
Source
 ↓
Lexer
 ↓
Parser
 ↓
AST
```

**Deliverable:** Valid causis programs can be parsed.

---

## Milestone 2 — Simulation

```text
World
 ↓
Agents
 ↓
Movement
 ↓
Collision
```

**Deliverable:** A C++ simulation runs independently.

---

## Milestone 3 — Compiler

```text
AST
 ↓
Semantic Analysis
 ↓
IR
 ↓
Bytecode
```

**Deliverable:** causis programs compile.

---

## Milestone 4 — VM

```text
Bytecode
 ↓
VM
 ↓
Simulation
```

**Deliverable:** Compiled programs execute.

---

## Milestone 5 — Visualization

```text
VM
 ↓
Simulation
 ↓
2D Grid
```

**Deliverable:** A program visibly controls a simulation.

**This is now the real finish line for the working plan (see Section 0).
Milestones 6-7 below are cut for this cycle and kept only for reference.**

---

## Milestone 6 — Intelligent Frontend **[CUT — see Section 0]**

```text
Natural Language
 ↓
LLM
 ↓
causis
 ↓
Compiler
 ↓
Simulation
```

**Deliverable:** Users can describe simulations naturally.

---

## Milestone 7 — Final System **[CUT — see Section 0]**

```text
Natural Language
      ↓
     LLM
      ↓
 causis Source
      ↓
     Lexer
      ↓
    Parser
      ↓
Semantic Analysis
      ↓
      IR
      ↓
  Optimization
      ↓
   Bytecode
      ↓
      VM
      ↓
  Simulation
      ↓
 Visualization
```

**Deliverable:** Complete end-to-end system.

---

# 35. Estimated Timeline

## Original full-scope estimate (reference only)

Assuming one student working approximately 2–3 hours/day:

| Stage | Estimated Time |
|---|---:|
| Project setup | 1–2 days |
| Language specification | 3–4 days |
| Lexer | 4–5 days |
| Parser + AST | 7–10 days |
| Semantic analysis | 5–7 days |
| Simulation engine | 4–7 days |
| Simulation semantics | 3–5 days |
| IR | 5–7 days |
| Optimization | 4–6 days |
| Bytecode compiler | 4–6 days |
| VM | 5–7 days |
| CLI | 2–3 days |
| Visualization | 7–10 days |
| Pathfinding | 3–5 days |
| LLM integration | 3–5 days |
| LLM correction loop | 2–4 days |
| Testing + benchmarking | 5–7 days |
| Documentation + polish | 5–7 days |

### Total (original)

**Approximately 10–14 weeks for one student.**

With a team of 2–3 students:

**approximately 6–9 weeks**, depending on how much polish is required.

## Current working estimate **[see Section 0]**

Realistic pace given this runs parallel to placement prep (OA rounds land
with little notice): roughly **1–2 hours/day on average**, not 2–3.

| Stage | Estimated Time |
|---|---:|
| Project setup | 1–2 days |
| Language specification | 3–4 days |
| Lexer | 4–5 days |
| Parser + AST | 7–10 days |
| Semantic analysis | 5–7 days |
| Simulation engine | 4–7 days |
| Simulation semantics | 3–5 days |
| IR | 5–7 days |
| Optimization (minimal — folding + DCE only) | 1–2 days |
| Bytecode compiler | 4–6 days |
| VM | 5–7 days |
| CLI | 2–3 days |
| Visualization (static tick-log viewer) | 2–4 days |
| Testing + documentation | 4–6 days |

**LLM integration and LLM correction loop: removed from the estimate —
not part of this cycle.**

### Total (current)

**Roughly 6–8 weeks of pipeline work**, and likely 16–20 weeks of
*elapsed calendar time* at 1–2 hrs/day once OA-prep interruptions are
factored in. Pathfinding and Compiler Visualization (Stage 13) are
optional add-ons beyond this if time allows after Milestone 4/5.

---

# 36. Priority System **[revised — see Section 0]**

If time becomes limited:

## P0 — Must Have (this is the current Definition of Done)

```text
Lexer
Parser
AST
Semantic Analysis
Simulation
IR
Minimal optimization (constant folding + dead-code elimination)
Bytecode
VM
Basic static visualization (tick-log viewer)
```

## P1 — Strongly Recommended (only after P0 is solid)

```text
Pathfinding
Professional errors
Step execution controls (upgrade from static viewer)
```

## P2 — Optional Stretch (only if far ahead of schedule)

```text
Compiler visualization (Stage 13 multi-pane pipeline view)
Additional optimizations (unreachable-code, redundant-op elimination)
Multiple simulation scenarios
Benchmarking / performance comparison
```

## Cut for this cycle — not planned

```text
LLM generation (Stage 16)
LLM correction loop (Stage 17)
LLM evaluation (Stage 18)
```

Never sacrifice the compiler core (P0) to chase P1/P2 items.

---

# 37. Definition of Done **[revised — see Section 0]**

## Current working Definition of Done

The project is considered complete for this cycle when the program below
works end-to-end through **Tokenization → Parsing → AST → Semantic
Validation → IR → (minimal) Optimization → Bytecode → VM Execution →
Simulation → static tick-log Visualization**. The natural-language/LLM
paragraph below the code block is **not** part of this cycle's Definition
of Done — it's kept as the original stretch vision.

The project is considered complete when the following program works:

```text
world 20 20;

robot R at 2 2;
target T at 17 17;

obstacle at 8 5;
obstacle at 8 6;
obstacle at 8 7;
obstacle at 8 8;

behavior R {

    every tick {

        if obstacle_ahead {
            turn_right();
        }

        move_toward(T);
    }
}
```

and the system can perform:

```text
Source
 ↓
Tokenization
 ↓
Parsing
 ↓
AST
 ↓
Semantic Validation
 ↓
IR
 ↓
Optimization
 ↓
Bytecode
 ↓
VM Execution
 ↓
Simulation
 ↓
2D Visualization
```

**[CUT for this cycle — see Section 0]** Additionally, the original vision
had a user provide a request like the one below, with an LLM generating
a causis program validated by the compiler before execution. This is kept
as a stretch goal only, not part of the current Definition of Done:

```text
"Create a robot on the top left that reaches
the target on the bottom right while avoiding
the wall in the middle."
```

---

# 38. Final Project Positioning **[revised — see Section 0]**

## Current positioning

> **causis: A Domain-Specific Language and Compiler for 2D Grid Simulation**

The core technical contribution is:

```text
DSL
+
Compiler
+
Intermediate Representation
+
Optimization (minimal)
+
Bytecode VM
+
Simulation Runtime
+
Basic 2D Visualization
```

This alone is a complete, presentable compiler + simulation project:

```text
       causis DSL
              ↓
           Compiler
              ↓
        Optimized IR
              ↓
           Bytecode
              ↓
             VM
              ↓
        Simulation
              ↓
        Visualization
```

## Original stretch positioning (reference only, not current scope)

The original plan positioned this as:

> **causis: An LLM-Assisted Domain-Specific Language and Compiler for Interactive 2D Simulation**

with an LLM natural-language frontend layered on top (Stages 16-18,
Milestones 6-7). That layer is cut for this cycle (see Section 0). If
revisited later, the original framing still holds: the project would
remain **a compiler and simulation project with an LLM-powered frontend**,
not an LLM project with a compiler attached — the LLM would only generate
causis source, never control the simulation directly.