# AGENTS.md — causis project rules

These rules apply to every task in this repo, in every Cursor session.

## Context

causis is a student compiler project: a DSL + compiler + bytecode VM for
2D grid simulations, built in C++20 with CMake, following the stages laid
out in PLAN.md. It is a learning project, not a production system —
optimize for clarity and my ability to understand and explain the code,
not for cleverness.

## Code style

- Write C++ the way a solid 3rd-year CS student would: clear, idiomatic
  C++20, standard STL usage. Avoid heavy template metaprogramming,
  obscure macros, or "one-liner" tricks I wouldn't be able to explain
  line-by-line in a review or a viva.
- Prefer straightforward, well-named functions and small classes over
  abstraction layers I don't need yet (no premature interfaces/factories).
- Follow the stage order and file layout in PLAN.md
  (lexer -> parser -> ast -> semantic -> ir -> bytecode -> vm -> runtime).
- Match the existing style in the file you're editing rather than
  introducing a new pattern for the same problem.

## Explaining changes

- After any code change, summarize in plain language:
  - What you added or changed, and in which files.
  - Why you made that choice (briefly — one or two sentences is fine).
  - Anything I should double check or that's a placeholder/TODO.
- If you had to make a design decision that isn't specified in PLAN.md,
  call it out explicitly rather than silently picking one.

## Git

- Never run `git commit`, `git push`, `git tag`, or any command that
  changes repo history or remote state. I commit manually myself.
- You can run `git status`, `git diff`, `git log`, and other read-only
  git commands freely.
- If you think something is ready to commit, say so and stop — don't
  commit it.

## Scope discipline

- Do not implement the LLM natural-language-to-DSL layer (previously
  Milestones 6-7) unless I explicitly ask for it — it's out of scope
  for now.
- Keep the optimizer minimal (e.g. basic constant folding / dead-code
  elimination) rather than building out a full optimization pipeline.
- Keep visualization simple: a static HTML/Canvas page that reads a
  tick-log and steps through it. No live VM connection, no animation
  polish, no styling beyond making the grid readable.
