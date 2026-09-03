# causis

causis is a domain-specific language and compiler for describing, running, and visualizing deterministic 2D grid-based simulations.

## Current status

- **Stage 0** — project scaffold (CMake, directory layout, CLI help)
- **Stage 1** — v1 language specification complete
- **Next** — Stage 2 lexer

See [PLAN.md](PLAN.md) for the full roadmap and [docs/language.md](docs/language.md) for the v1 language spec.

## Examples

| File | Description |
|------|-------------|
| [examples/basic_move.ls](examples/basic_move.ls) | Robot moves right every tick |
| [examples/collision.ls](examples/collision.ls) | Robot turns when blocked ahead |
| [examples/target.ls](examples/target.ls) | Robot moves toward a named target |

## Build (MSYS2 UCRT64)

Use the **MSYS2 UCRT64** terminal (not plain PowerShell — `cmake` and `g++` live
in the MSYS environment). From the project root:

```sh
cmake -S . -B build -G Ninja
cmake --build build
```

If `build/` was configured with a different generator before, delete it first:

```sh
rm -rf build
```

From **PowerShell**, either open MSYS2 UCRT64, or prefix PATH for one session:

```powershell
$env:PATH = "C:\msys64\ucrt64\bin;C:\msys64\usr\bin;" + $env:PATH
cmake -S . -B build -G Ninja
cmake --build build
```

Adjust `C:\msys64` if your MSYS2 install is elsewhere.

## Smoke Test

```sh
ctest --test-dir build --output-on-failure
```

## CLI

`causis` is not installed globally. Run the built binary:

```sh
./build/causis.exe --help
```

In PowerShell:

```powershell
.\build\causis.exe --help
```
