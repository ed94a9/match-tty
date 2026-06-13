# Fall-Down Animation

## Overview

After match elimination removes pins from the grid, remaining pins fall down to fill gaps and new pins enter from above. This is animated over 8 frames using pixel-level clipping via the `Clip` modifier.

## Architecture

### State Machine Integration

```
SwapForward → SwapBackward → Eliminate → FallDown → (chain)
                                      ↑                |
                                      └──── recurses ──┘
```

- `EliminateState` finishes → `finishElimination()` zeroes matched cells and creates `FallDownState`
- `FallDownState` animates pins falling → `handleStateDone` applies final board → `startElimination()` checks for chain reactions

### Key Components

| Component | Location | Role |
|-----------|----------|------|
| `FallDownState` | `include/match-tty/game/FallDownState.h` | AnimState that computes per-column packing and interpolates pin positions |
| `Move` struct | `FallDownState.h:16` | Records `(src_px, tgt_px, col, value)` for each moving pin |
| `Clip` modifier | `include/match-tty/modifiers/clip.h` | Pre-renders an element and extracts a pixel sub-rectangle |
| `VerticalStackMerger` | `include/match-tty/assets/components.h` | Merges two clipped pin elements in one cell (unused; Clip is used directly) |

### How It Works

**Constructor** — For each column independently:
1. Scan bottom-up, collect non-zero (surviving) pins with original positions
2. Pack surviving pins to the bottom of the column
3. Generate new random pins at the top
4. For each pin that moves, create a `Move` with pixel-space start/end positions
5. Store the final post-fall board in `final_board_`

**Rendering** (`renderPin`) — For each grid cell `(r,c)` at frame `f`:
1. Compute each overlapping Move's interpolated pixel position:  
   `pin_px = src_px + (tgt_px - src_px) * f / 8`
2. For each pin overlapping the cell's 5-pixel range: compute the overlap as a `CellSlice` (pin-local rows, cell-local rows)
3. Sort slices top-to-bottom by cell-local position
4. Build a `vbox` of clipped pin elements with spacers filling gaps
5. If no falling pin overlaps: show the original board value (or empty 5×5 placeholder for gaps/vacated cells)

### Value Encoding

Tile values are `1..6` (inclusive). Value `0` is exclusively a **gap marker** used temporarily during elimination. This avoids ambiguity where `0` could mean both "tile type 0" and "empty cell."

- `make_pin_anyway(v)`: maps `v` ∈ [1,6] → one of 6 pin visuals (value `≤0` treated as gap → pin 2)
- `defaultGenerate()`: returns `dist(1, 6)` — never generates `0`
- Initial board: `((r * c) % 6) + 1`

## Debug Logging

Key transitions log via `QLOG_INFO`:

| Event | Log message |
|-------|-------------|
| FallDownState constructed | `FallDownState created: N moves` |
| Per-column pin fall | `[col N] pin val=N falls X->Y` |
| Per-column new pin | `[col N] new val=N falls from above->Y` |
| Elimination cleanup | `finishElimination: N pins eliminated, bonus time=N` |
| FallDown complete | `FallDown complete, final board applied` |
| Match found | `Found N matched tiles, eliminating... (from_swap=N)` |

## Build

Add `FallDownState.cpp` to `src/CMakeLists.txt` and add `Clip` modifier with `#pragma once` + `inline`.
