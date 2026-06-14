# match-tty

A **Match-3 puzzle game** that runs entirely in your terminal, built with modern C++20 and [FTXUI](https://github.com/ArthurSonzogni/FTXUI).

Swap adjacent pin tiles to match rows or columns of 3+ identical tiles, earn points, and beat the clock.

## Features

- **6 tile types** rendered as dice-like dot patterns
- **Smooth animations**: swap, eliminate, fall-down (gravity), and flash effects
- **Chain reactions**: matches trigger cascading eliminations
- **Countdown timer** with color-coded time bar
- **Scoring** with level progression
- **Swap penalty**: rapid swaps cost time
- **Welcome & game-over screens** with ASCII art

## Usage

```bash
./src/match-tty --rows 7 --cols 19 --dur 60
```

Use **arrow keys** or **WASD** to navigate, **Space** to select, and **Space** again on an adjacent tile to swap.

| Option | Description |
|---|---|
| `--rows`, `--cols` | Grid dimensions |
| `-d`, `--dur` | Game time in seconds (default: 60) |
| `--time-gain` | Time bonus per eliminated pin |
| `--penalty` | Penalty seconds for rapid swaps |
| `--log <file>` | Enable file logging |
| `-f`, `--frame-dur-ms` | Animation frame rate (dev) |

## Build

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

All dependencies (FTXUI, fmt, Quill, Lyra, Catch2) are fetched automatically via CMake `FetchContent`.

## Run Tests

```bash
./tests/test_match
```

## Project

```
src/          — Game logic, CLI, menu screens
include/      — Headers: algorithms, assets, game states, utilities
tests/        — Catch2 unit tests
examples/     — FTXUI examples
cmake/        — Custom Find modules for dependencies
```
