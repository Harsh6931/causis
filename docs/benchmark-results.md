# Benchmark results

Measured on the development machine (Windows, MSYS2 UCRT64, GCC C++20) with:

```text
./build/causis_bench.exe suite --iterations 20
```

The harness reports **median** compile-stage times over repeated runs (one warmup compile, then timed iterations). VM timings compile the program once, then time only `run_bytecode()` for the requested tick count.

## Compile pipeline (median ms)

| Program | tokenize | parse | semantic | lower | optimize | bytecode | total |
|---------|----------|-------|----------|-------|----------|----------|-------|
| `examples/basic_move.ls` | 0.029 | 0.017 | 0.007 | 0.015 | 0.026 | 0.009 | 0.101 |
| `examples/target.ls` | 0.036 | 0.023 | 0.010 | 0.017 | 0.028 | 0.010 | 0.124 |
| `examples/path_to_target.ls` | 0.111 | 0.062 | 0.024 | 0.050 | 0.076 | 0.021 | 0.343 |

### Takeaway

For the largest example (`path_to_target.ls`), **tokenize** and **optimize** are the two slowest stages; semantic analysis and bytecode emission stay comparatively cheap. All stages remain sub-millisecond on these v1 programs.

## VM throughput (`path_to_target.ls`, optimized bytecode)

| Ticks | Wall time (ms) | Throughput (ticks/sec) |
|-------|----------------|-------------------------|
| 1,000 | 0.961 | ~1.04M |
| 10,000 | 9.624 | ~1.04M |
| 100,000 | 93.877 | ~1.07M |

Throughput is stable across tick counts, which is expected for a tight interpreter loop over a fixed bytecode program.

## Optimized vs raw IR bytecode

Same program, VM-only timing (`--compare-opt`):

| Ticks | optimized (ms) | raw IR (ms) |
|-------|----------------|-------------|
| 1,000 | 0.961 | 0.991 |
| 10,000 | 9.624 | 9.206 |
| 100,000 | 93.877 | 96.860 |

IR optimization here removes a small number of instructions (see [optimization-results.md](optimization-results.md)); **VM time differences are within measurement noise** on this hardware. The optimizer’s main benefit for v1 is smaller IR/bytecode and clearer `--stats` reporting, not large runtime speedups on tiny programs.

## End-to-end snapshot (`path_to_target.ls`, 1000 ticks)

| Phase | Time (ms) |
|-------|-----------|
| Full compile pipeline | 0.416 |
| VM (optimized) | 0.905 |
| VM (raw IR bytecode) | 0.917 |
| Total (compile + optimized VM) | ~1.32 |

## Running the harness locally

```text
causis_bench compile <file.ls> [--iterations N]
causis_bench vm <file.ls> [--ticks N] [--compare-opt]
causis_bench e2e <file.ls> [--ticks N] [--compare-opt]
causis_bench suite [--iterations N]
```

The `causis_bench` target is built with the main project (`cmake --build build`); it is not registered in CTest.
