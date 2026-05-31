#pragma once

#include <vector>
#include <chrono>
#include <utility>
#include <functional>
#include <random>
#include <memory>
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <match-tty/assets/mj-8pins.h>
#include <match-tty/algo/match.h>
#include <match-tty/game/TimeBar.h>
#include <match-tty/utils/Logger.h>
#include <match-tty/game/AnimState.h>

class GameBoardState
{
public:
    static constexpr size_t max_rows = 7;
    static constexpr size_t max_cols = 19;

    using GenerateCallback = std::function<int(size_t, size_t)>;

    struct ElimPin {
        size_t row, col;
        int delay;
    };

private:
    // ── Data ─────────────────────────────────────────────────────────
    std::vector<std::vector<int>> board_state_;
    GenerateCallback generate_cb_;
    bool auto_swap_back_ = false;

    std::unique_ptr<AnimState> current_state_;

    std::pair<size_t, size_t> src_tile_{0, 0}, tgt_tile_{0, 0};

    static constexpr int elim_hold_ticks_ = 7;
    std::vector<ElimPin> eliminating_pins_;

    std::chrono::milliseconds frame_duration_{0};
    std::chrono::steady_clock::time_point last_frame_time_;

    TimeBar* time_bar_ = nullptr;

    // ── Internal helpers ─────────────────────────────────────────────
    void handleStateDone();
    void startElimination(bool from_swap = false);
    void finishElimination();
    static int defaultGenerate(size_t, size_t);

public:
    // ── Public accessors for state classes ───────────────────────────
    int tileAt(size_t r, size_t c) const { return board_state_[r][c]; }
    std::pair<size_t, size_t> srcTile() const { return src_tile_; }
    std::pair<size_t, size_t> tgtTile() const { return tgt_tile_; }
    const std::vector<ElimPin>& eliminatingPins() const { return eliminating_pins_; }
    static constexpr int elimHoldTicks() { return elim_hold_ticks_; }

    // ── Construction ─────────────────────────────────────────────────
    GameBoardState(int frame_dur_ms, bool auto_swap_back = false,
                   GenerateCallback gen_cb = defaultGenerate)
        : frame_duration_(frame_dur_ms)
        , last_frame_time_(std::chrono::steady_clock::now())
        , generate_cb_(std::move(gen_cb))
        , auto_swap_back_(auto_swap_back)
    {
        board_state_.resize(max_rows, std::vector<int>(max_cols, 0));
        for (size_t r = 0; r < max_rows; ++r)
            for (size_t c = 0; c < max_cols; ++c)
                board_state_[r][c] = (r * c) % 6;
        startElimination();
    }

    // ── Game loop interface ──────────────────────────────────────────
    void UpdateAnimationTimeline(ftxui::ScreenInteractive&) {
        if (!current_state_) return;

        auto now = std::chrono::steady_clock::now();
        if (now - last_frame_time_ < frame_duration_)
            return;

        last_frame_time_ = now;

        if (!current_state_->advance())
            handleStateDone();
    }

    void TryInsertNextFrame(ftxui::ScreenInteractive& screen) {
        if (current_state_)
            screen.PostEvent(ftxui::Event::Custom);
    }

    void TriggerSwap(std::pair<size_t, size_t> source,
                     std::pair<size_t, size_t> target,
                     ftxui::ScreenInteractive& screen);

    ftxui::Element RenderPin(size_t r, size_t c,
                             bool is_focused, bool is_activated) const {
        ftxui::Element pin;

        if (current_state_)
            pin = current_state_->renderPin(r, c);

        if (!pin)
            pin = mtty::make_pin_anyway(board_state_[r][c]);

        if (is_activated)
            pin = pin | ftxui::bgcolor(ftxui::Color::Red)
                      | ftxui::color(ftxui::Color::White);
        else if (is_focused)
            pin = pin | ftxui::bgcolor(ftxui::Color::GrayDark);

        return pin;
    }

    bool isAnimating() const { return current_state_ != nullptr; }
    bool isEliminating() const {
        return current_state_ && current_state_->type() == AnimType::Eliminate;
    }
    bool isVerticalSwap() const {
        return current_state_ &&
              (current_state_->type() == AnimType::SwapForward ||
               current_state_->type() == AnimType::SwapBackward) &&
              src_tile_.first != tgt_tile_.first;
    }
    bool isHorizontalSwap() const {
        return current_state_ &&
              (current_state_->type() == AnimType::SwapForward ||
               current_state_->type() == AnimType::SwapBackward) &&
              src_tile_.second != tgt_tile_.second;
    }

    void setTimeBar(TimeBar* tb) { time_bar_ = tb; }
};


