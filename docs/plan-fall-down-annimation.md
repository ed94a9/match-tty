# Plan: Fall-Down Animation

## Summary
Add a "fall-down" animation for pins after match elimination. This involves creating a `VerticalStackMerger` to allow pins to visually slide between grid rows by "stacking" fractional parts of pin elements.

## Key Changes
- **Merger System**: Introduce `mtty::Merger` interface and `mtty::VerticalStackMerger`. The merger will take two pin elements and stack the top portion of the upper pin onto the bottom portion of the lower pin.
- **Slicing Logic**: Implement a utility to "slice" a pin's `ftxui::Element` vertically based on a row count.
- **FallDownState**: A new `AnimState` that calculates a fractional row for each pin in a column, interpolates between the current and target row, and uses `VerticalStackMerger` to render the transition.
- **State Machine Update**: Modify `GameBoardState::handleStateDone` to transition from `EliminateState` to `FallDownState` before calling `finishElimination`.

## Test Plan
- Verify that the fall-down animation triggers for all columns after elimination.
- Confirm that pins move smoothly (fractional rows) rather than jumping.
- Ensure that `finishElimination` (and the subsequent new pin generation) only occurs after the fall-down animation completes.

## Assumptions
- Each pin is composed of a fixed number of rows (e.g., 3 or 5).
- The `VerticalStackMerger` will assume a standard grid where pins are centered.
- The animation will run at the same `frame_duration_` as the rest of the game.
