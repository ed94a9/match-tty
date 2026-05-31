#pragma once

#include <vector>
#include <chrono>
#include <utility>
#include <algorithm>
#include <string>
#include <functional>
#include <random>
#include <memory>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <match-tty/algo/match.h>
#include <match-tty/game/TimeBar.h>
#include <match-tty/utils/Logger.h>

class GameBoardState
{
public:
    static constexpr size_t max_rows = 7;
    static constexpr size_t max_cols = 19;

    using GenerateCallback = std::function<int(size_t, size_t)>;

private:
    // ── AnimState hierarchy ──────────────────────────────────────────
    enum class AnimType { SwapForward, SwapBackward, Eliminate, Flash };

    struct AnimState {
        virtual ~AnimState() = default;
        virtual AnimType type() const = 0;
        /// Advance one step. Returns true if more steps remain, false when done.
        virtual bool advance() = 0;
        /// Render a single pin. Returns nullptr if this state doesn't touch (r,c).
        virtual ftxui::Element renderPin(size_t r, size_t c) const = 0;
    };

    struct SwapForwardState final : AnimState {
        GameBoardState& g;
        int frame = 0;
        explicit SwapForwardState(GameBoardState& g) : g(g) {}
        AnimType type() const override { return AnimType::SwapForward; }
        bool advance() override;
        ftxui::Element renderPin(size_t r, size_t c) const override;
    };

    struct SwapBackwardState final : AnimState {
        GameBoardState& g;
        int frame = 4;
        explicit SwapBackwardState(GameBoardState& g) : g(g) {}
        AnimType type() const override { return AnimType::SwapBackward; }
        bool advance() override;
        ftxui::Element renderPin(size_t r, size_t c) const override;
    };

    struct EliminateState final : AnimState {
        GameBoardState& g;
        int frame = 0;
        explicit EliminateState(GameBoardState& g) : g(g) {}
        AnimType type() const override { return AnimType::Eliminate; }
        bool advance() override;
        ftxui::Element renderPin(size_t r, size_t c) const override;
    };

    struct FlashState final : AnimState {
        GameBoardState& g;
        int counter = 4;
        explicit FlashState(GameBoardState& g) : g(g) {}
        AnimType type() const override { return AnimType::Flash; }
        bool advance() override;
        ftxui::Element renderPin(size_t r, size_t c) const override;
    };

    // ── Helpers ──────────────────────────────────────────────────────
    void handleStateDone();
    void startElimination(bool from_swap = false);
    void finishElimination();
    static int defaultGenerate(size_t, size_t);

    // ── Data ─────────────────────────────────────────────────────────
    std::vector<std::vector<int>> board_state_;
    GenerateCallback generate_cb_;
    bool auto_swap_back_ = false;

    // Current animation (nullptr = idle)
    std::unique_ptr<AnimState> current_state_;

    // Shared by swap states
    std::pair<size_t, size_t> src_tile_{0, 0}, tgt_tile_{0, 0};

    // Shared by elimination
    static constexpr int elim_hold_ticks_ = 7;
    struct ElimPin {
        size_t row, col;
        int delay;
    };
    std::vector<ElimPin> eliminating_pins_;

    // Frame throttle
    std::chrono::milliseconds frame_duration_{0};
    std::chrono::steady_clock::time_point last_frame_time_;

    // External dependencies
    TimeBar* time_bar_ = nullptr;

public:
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

    /// Advance one animation step (called once per render cycle).
    void UpdateAnimationTimeline(ftxui::ScreenInteractive&) {
        if (!current_state_) return;

        auto now = std::chrono::steady_clock::now();
        if (now - last_frame_time_ < frame_duration_)
            return;

        last_frame_time_ = now;

        if (!current_state_->advance())
            handleStateDone();
    }

    /// If animation is still active, request another render.
    void TryInsertNextFrame(ftxui::ScreenInteractive& screen) {
        if (current_state_)
            screen.PostEvent(ftxui::Event::Custom);
    }

    void TriggerSwap(std::pair<size_t, size_t> source,
                     std::pair<size_t, size_t> target,
                     ftxui::ScreenInteractive& screen) {
        if ((time_bar_ && time_bar_->isOver()) || current_state_)
            return;

        src_tile_ = source;
        tgt_tile_ = target;

        QLOG_INFO("Swap ({},{}) <-> ({},{})",
                  source.first, source.second,
                  target.first, target.second);

        last_frame_time_ = std::chrono::steady_clock::now();
        current_state_ = std::make_unique<SwapForwardState>(*this);
        TryInsertNextFrame(screen);
    }

    ftxui::Element RenderPin(size_t r, size_t c,
                             bool is_focused, bool is_activated) const {
        ftxui::Element pin;

        if (current_state_) {
            pin = current_state_->renderPin(r, c);
        }

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

// ── inline implementations ─────────────────────────────────────────────

inline void GameBoardState::handleStateDone() {
    switch (current_state_->type()) {
    case AnimType::SwapForward: {
        std::swap(board_state_[src_tile_.first][src_tile_.second],
                  board_state_[tgt_tile_.first][tgt_tile_.second]);
        current_state_ = std::make_unique<SwapBackwardState>(*this);
        break;
    }
    case AnimType::SwapBackward: {
        current_state_.reset();
        startElimination(true);
        break;
    }
    case AnimType::Eliminate: {
        current_state_.reset();
        finishElimination();
        break;
    }
    case AnimType::Flash: {
        current_state_.reset();
        break;
    }
    }
}

// ── AnimState implementations ─────────────────────────────────────────

inline bool GameBoardState::SwapForwardState::advance() {
    if (++frame < 4) return true;
    return false; // done, handleStateDone swaps data
}

inline ftxui::Element
GameBoardState::SwapForwardState::renderPin(size_t r, size_t c) const {
    bool is_src = (r == g.src_tile_.first && c == g.src_tile_.second);
    bool is_tgt = (r == g.tgt_tile_.first && c == g.tgt_tile_.second);
    bool in_src_col = (c == g.src_tile_.second);
    bool in_tgt_col = (c == g.tgt_tile_.second);
    bool h_swap = (g.src_tile_.second != g.tgt_tile_.second);
    int sr = static_cast<int>(g.src_tile_.first);
    int tr = static_cast<int>(g.tgt_tile_.first);
    int sc = static_cast<int>(g.src_tile_.second);
    int tc = static_cast<int>(g.tgt_tile_.second);

    ftxui::Element pin = mtty::make_pin_anyway(g.board_state_[r][c]);

    if (h_swap) {
        if (tc > sc) {
            if (in_tgt_col) pin = pin | ftxui::align_right;
            else if (in_src_col) pin = ftxui::hbox({ pin, ftxui::filler() });
        } else {
            if (in_src_col) pin = pin | ftxui::align_right;
            else if (in_tgt_col) pin = ftxui::hbox({ pin, ftxui::filler() });
        }
    } else {
        if (tr > sr) {
            if (r == static_cast<size_t>(tr))
                pin = ftxui::vbox({ ftxui::filler(), pin });
            else if (r == static_cast<size_t>(sr))
                pin = ftxui::vbox({ pin, ftxui::filler() });
        } else {
            if (r == static_cast<size_t>(sr))
                pin = ftxui::vbox({ ftxui::filler(), pin });
            else if (r == static_cast<size_t>(tr))
                pin = ftxui::vbox({ pin, ftxui::filler() });
        }
    }

    // Animated sliding
    if (is_src || is_tgt) {
        int r_diff = tr - sr;
        int c_diff = tc - sc;
        int shift = frame;
        int val = g.board_state_[r][c];

        if (is_tgt) { r_diff = -r_diff; c_diff = -c_diff; }

        if (c_diff != 0) {
            pin = c_diff > 0
                ? ftxui::hbox({ ftxui::text(std::string(shift, ' ')),
                                mtty::make_pin_anyway(val),
                                ftxui::text(std::string(4 - shift, ' ')) })
                : ftxui::hbox({ ftxui::text(std::string(4 - shift, ' ')),
                                mtty::make_pin_anyway(val),
                                ftxui::text(std::string(shift, ' ')) });
        } else if (r_diff != 0) {
            pin = r_diff > 0
                ? ftxui::vbox({ mtty::vertical_spacer(shift),
                                mtty::make_pin_anyway(val),
                                mtty::vertical_spacer(4 - shift) })
                : ftxui::vbox({ mtty::vertical_spacer(4 - shift),
                                mtty::make_pin_anyway(val),
                                mtty::vertical_spacer(shift) });
        }
        return pin | ftxui::bgcolor(ftxui::Color::DarkBlue);
    }

    return pin;
}

inline bool GameBoardState::SwapBackwardState::advance() {
    if (--frame > 0) return true;
    return false;
}

inline ftxui::Element
GameBoardState::SwapBackwardState::renderPin(size_t r, size_t c) const {
    bool is_src = (r == g.src_tile_.first && c == g.src_tile_.second);
    bool is_tgt = (r == g.tgt_tile_.first && c == g.tgt_tile_.second);
    bool in_src_col = (c == g.src_tile_.second);
    bool in_tgt_col = (c == g.tgt_tile_.second);
    bool h_swap = (g.src_tile_.second != g.tgt_tile_.second);
    int sr = static_cast<int>(g.src_tile_.first);
    int tr = static_cast<int>(g.tgt_tile_.first);
    int sc = static_cast<int>(g.src_tile_.second);
    int tc = static_cast<int>(g.tgt_tile_.second);

    ftxui::Element pin = mtty::make_pin_anyway(g.board_state_[r][c]);

    if (h_swap) {
        if (tc > sc) {
            if (in_tgt_col) pin = pin | ftxui::align_right;
            else if (in_src_col) pin = ftxui::hbox({ pin, ftxui::filler() });
        } else {
            if (in_src_col) pin = pin | ftxui::align_right;
            else if (in_tgt_col) pin = ftxui::hbox({ pin, ftxui::filler() });
        }
    } else {
        if (tr > sr) {
            if (r == static_cast<size_t>(tr))
                pin = ftxui::vbox({ ftxui::filler(), pin });
            else if (r == static_cast<size_t>(sr))
                pin = ftxui::vbox({ pin, ftxui::filler() });
        } else {
            if (r == static_cast<size_t>(sr))
                pin = ftxui::vbox({ ftxui::filler(), pin });
            else if (r == static_cast<size_t>(tr))
                pin = ftxui::vbox({ pin, ftxui::filler() });
        }
    }

    if (is_src || is_tgt) {
        int r_diff = tr - sr;
        int c_diff = tc - sc;
        int shift = frame;
        int val = g.board_state_[r][c];

        if (is_tgt) { r_diff = -r_diff; c_diff = -c_diff; }

        if (c_diff != 0) {
            pin = c_diff > 0
                ? ftxui::hbox({ ftxui::text(std::string(shift, ' ')),
                                mtty::make_pin_anyway(val),
                                ftxui::text(std::string(4 - shift, ' ')) })
                : ftxui::hbox({ ftxui::text(std::string(4 - shift, ' ')),
                                mtty::make_pin_anyway(val),
                                ftxui::text(std::string(shift, ' ')) });
        } else if (r_diff != 0) {
            pin = r_diff > 0
                ? ftxui::vbox({ mtty::vertical_spacer(shift),
                                mtty::make_pin_anyway(val),
                                mtty::vertical_spacer(4 - shift) })
                : ftxui::vbox({ mtty::vertical_spacer(4 - shift),
                                mtty::make_pin_anyway(val),
                                mtty::vertical_spacer(shift) });
        }
        return pin | ftxui::bgcolor(ftxui::Color::DarkBlue);
    }

    return pin;
}

inline bool GameBoardState::EliminateState::advance() {
    ++frame;
    for (auto& pin : g.eliminating_pins_)
        if ((frame - pin.delay) / elim_hold_ticks_ < 2)
            return true;
    return false;
}

inline ftxui::Element
GameBoardState::EliminateState::renderPin(size_t r, size_t c) const {
    for (auto& pin : g.eliminating_pins_) {
        if (pin.row == r && pin.col == c) {
            int local = frame - pin.delay;
            if (local < 0) break;
            int visual = local / elim_hold_ticks_;
            if (visual == 0)
                return mtty::make_pin_center_only(g.board_state_[r][c])
                     | ftxui::bgcolor(ftxui::Color::DarkBlue);
            if (visual == 1)
                return mtty::make_center_dot()
                     | ftxui::bgcolor(ftxui::Color::DarkBlue);
            return ftxui::text(" ")
                 | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 1);
        }
    }
    return {};
}

inline bool GameBoardState::FlashState::advance() {
    return --counter > 0;
}

inline ftxui::Element
GameBoardState::FlashState::renderPin(size_t r, size_t c) const {
    if ((r == g.src_tile_.first && c == g.src_tile_.second) ||
        (r == g.tgt_tile_.first && c == g.tgt_tile_.second))
        return mtty::make_pin_anyway(g.board_state_[r][c])
             | ftxui::bgcolor(ftxui::Color::Cyan);
    return {};
}

// ── Game logic ───────────────────────────────────────────────────────

inline void GameBoardState::startElimination(bool from_swap) {
    auto matched = mtty::algo::check_match(board_state_);
    if (matched.empty()) {
        if (from_swap && auto_swap_back_) {
            std::swap(board_state_[src_tile_.first][src_tile_.second],
                      board_state_[tgt_tile_.first][tgt_tile_.second]);
            QLOG_INFO("No match, swapping back ({},{}) <-> ({},{})",
                      src_tile_.first, src_tile_.second,
                      tgt_tile_.first, tgt_tile_.second);
            current_state_ = std::make_unique<FlashState>(*this);
        }
        return;
    }

    QLOG_INFO("Found {} matched tiles, eliminating...", matched.size());

    eliminating_pins_.clear();
    for (auto& [r, c] : matched)
        eliminating_pins_.push_back({r, c, 0});

    current_state_ = std::make_unique<EliminateState>(*this);
}

inline void GameBoardState::finishElimination() {
    for (auto& pin : eliminating_pins_)
        board_state_[pin.row][pin.col] = generate_cb_(pin.row, pin.col);
    eliminating_pins_.clear();
    startElimination();
}

inline int GameBoardState::defaultGenerate(size_t, size_t) {
    static std::mt19937 rng{std::random_device{}()};
    static std::uniform_int_distribution<int> dist(0, 5);
    return dist(rng);
}
