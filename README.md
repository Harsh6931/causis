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
