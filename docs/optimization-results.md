# Optimization results

Measured on the development machine with:

```text
causis optimize examples/path_to_target.ls --stats
```

## path_to_target.ls

```text
Program: examples/path_to_target.ls

Optimization Statistics
-----------------------
IR instructions:
  Before       : 37
  After        : 36
  Removed      : 1
  Reduction    : 2.7%

Bytecode instructions:
  Before       : 32
  After        : 31
  Removed      : 1
  Reduction    : 3.1%

Passes applied:
  - constant-folding
  - branch-pruning
  - stop-unreachable
  - redundant-turn-elimination
```

## Notes

- v1 programs are small, so percentage gains are modest; the important part is that
`--stats` reports before/after/removed counts and reduction percentages.
- Pass B (constant branch pruning) is handled by the existing `eliminate_dead_branches`
pass together with constant folding.
- Pass A removes straight-line instructions after `stop()` + `jump` until the next label.
- Pass C removes opposite turn pairs (`turn_left` / `turn_right`) when separated only by
expression-statement `Pop` noise.

