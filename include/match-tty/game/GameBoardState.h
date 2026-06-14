#pragma once

#include <vector>
#include <deque>
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
#include <match-tty/game/ScoreBar.h>
#include <match-tty/utils/Logger.h>
#include <match-tty/game/AnimState.h>
#include <match-tty/game/GridGenerator.h>

class GameBoardState
{
public:
    using GenerateCallback = std::function<int(size_t, size_t)>;

    struct ElimPin {
        size_t row, col;
        int delay;
    };

private:
    size_t rows_ = 7;
    size_t cols_ = 19;

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
    ScoreBar* score_bar_ = nullptr;
    int time_gain_ = 1;
    int penalty_secs_ = 5;

    // ── Swap penalty tracking ────────────────────────────────────────
    std::deque<std::chrono::steady_clock::time_point> swap_timestamps_;
    std::string alert_message_;
    std::chrono::steady_clock::time_point alert_start_time_;
    int alert_duration_secs_ = 1;

    // ── Internal helpers ─────────────────────────────────────────────
    void handleStateDone();
    void startElimination(bool from_swap = false);
    void finishElimination();

public:
    static int defaultGenerate(size_t, size_t);

    // ── Public accessors for state classes ───────────────────────────
    size_t rows() const { return rows_; }
    size_t cols() const { return cols_; }
    int tileAt(size_t r, size_t c) const { return board_state_[r][c]; }
    int generateTile(size_t r, size_t c) { return generate_cb_(r, c); }
    std::pair<size_t, size_t> srcTile() const { return src_tile_; }
    std::pair<size_t, size_t> tgtTile() const { return tgt_tile_; }
    const std::vector<ElimPin>& eliminatingPins() const { return eliminating_pins_; }
    static constexpr int elimHoldTicks() { return elim_hold_ticks_; }

    // ── Construction ─────────────────────────────────────────────────
    GameBoardState(size_t rows, size_t cols, int frame_dur_ms,
                   bool auto_swap_back = false,
                   GenerateCallback gen_cb = defaultGenerate,
                   int time_gain = 1,
                   int penalty_secs = 5,
                   std::unique_ptr<GridGenerator> grid_gen = {})
        : rows_(rows), cols_(cols)
        , frame_duration_(frame_dur_ms)
        , last_frame_time_(std::chrono::steady_clock::now())
        , generate_cb_(std::move(gen_cb))
        , auto_swap_back_(auto_swap_back)
        , time_gain_(time_gain)
        , penalty_secs_(penalty_secs)
    {
        if (!grid_gen)
            grid_gen = std::make_unique<MatchFreeGridGenerator>();
        board_state_ = grid_gen->generate(rows_, cols_);
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
    void setScoreBar(ScoreBar* sb) { score_bar_ = sb; }

    // ── Alert / penalty public interface ─────────────────────────────
    std::string getAlert() const {
        if (alert_message_.empty()) return {};
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::steady_clock::now() - alert_start_time_).count();
        if (elapsed >= alert_duration_secs_)
            return {};
        return alert_message_;
    }
};


