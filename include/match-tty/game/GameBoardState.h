#pragma once

#include <vector>
#include <chrono>
#include <utility>
#include <algorithm>
#include <string>
#include <functional>
#include <random>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <match-tty/algo/match.h>
#include <match-tty/game/TimeBar.h>
#include <match-tty/utils/Logger.h>

class GameBoardState
{
public:
    static constexpr size_t max_rows = 7;
    static constexpr size_t max_cols = 19;

    using GenerateCallback = std::function<int(size_t, size_t)>;

    GameBoardState(int frame_dur_ms, bool auto_swap_back = false, GenerateCallback gen_cb = defaultGenerate)
        : is_animating(false)
        , anim_frame(0)
        , is_fading_back(false)
        , frame_duration(frame_dur_ms)
        , last_frame_time(std::chrono::steady_clock::now())
        , board_state(max_rows, std::vector<int>(max_cols, 0))
        , generate_cb_(std::move(gen_cb))
        , is_eliminating_(false)
        , elim_global_frame_(0)
        , auto_swap_back_(auto_swap_back)
    {
        for (size_t r = 0; r < max_rows; ++r) {
            for (size_t c = 0; c < max_cols; ++c) {
                board_state[r][c] = (r * c) % 6;
            }
        }
        startElimination();
    }

    void UpdateAnimationTimeline(ftxui::ScreenInteractive& screen) {
        if (!is_animating && !is_eliminating_ && swap_back_flash_ == 0) return;

        auto now = std::chrono::steady_clock::now();
        if (now - last_frame_time >= frame_duration) {
            last_frame_time = now;

            if (is_animating) {
                if (!is_fading_back) {
                    if (anim_frame < 4) {
                        anim_frame++;
                    } else {
                        std::swap(board_state[src_tile.first][src_tile.second],
                                  board_state[tgt_tile.first][tgt_tile.second]);
                        is_fading_back = true;
                    }
                } else {
                    if (anim_frame > 0) {
                        anim_frame--;
                    } else {
                        is_animating = false;
                        startElimination(true);
                    }
                }
            }

            if (is_eliminating_) {
                elim_global_frame_++;

                bool all_done = true;
                for (auto& pin : eliminating_pins_) {
                    if ((elim_global_frame_ - pin.delay) / elim_hold_ticks_ < 2) {
                        all_done = false;
                        break;
                    }
                }

                if (all_done) {
                    is_eliminating_ = false;
                    finishElimination();
                }
            }
        }

        if (swap_back_flash_ > 0) {
            --swap_back_flash_;
        }

    }

    void TryInsertNextFrame(ftxui::ScreenInteractive& screen) {
        if (is_animating || is_eliminating_ || swap_back_flash_ > 0) {
            screen.PostEvent(ftxui::Event::Custom);
        }
    }

    void TriggerSwap(std::pair<size_t, size_t> source, std::pair<size_t, size_t> target, ftxui::ScreenInteractive& screen) {
        if ((time_bar_ && time_bar_->isOver()) || is_animating || is_eliminating_) return;

        is_animating = true;
        anim_frame = 0;
        is_fading_back = false;
        src_tile = source;
        tgt_tile = target;
        last_frame_time = std::chrono::steady_clock::now();

        QLOG_INFO("Swap ({},{}) <-> ({},{})", source.first, source.second, target.first, target.second);

        screen.PostEvent(ftxui::Event::Custom);
    }

    ftxui::Element RenderPin(size_t r, size_t c, bool is_focused, bool is_activated) const {
        if (is_eliminating_) {
            for (auto& pin : eliminating_pins_) {
                if (pin.row == r && pin.col == c) {
                    int local = elim_global_frame_ - pin.delay;
                    if (local < 0) break;
                    int visual = local / elim_hold_ticks_;
                    if (visual == 0) return mtty::make_pin_center_only(board_state[r][c]) | ftxui::bgcolor(ftxui::Color::DarkBlue);
                    if (visual == 1) return mtty::make_center_dot() | ftxui::bgcolor(ftxui::Color::DarkBlue);
                    return ftxui::text(" ") | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 1);
                }
            }
        }

        ftxui::Element pin = mtty::make_pin_anyway(board_state[r][c]);

        bool in_src_col = (c == src_tile.second);
        bool in_tgt_col = (c == tgt_tile.second);
        bool is_horizontal_swap = is_animating && (src_tile.second != tgt_tile.second);

        if (is_horizontal_swap) {
            int c_diff = static_cast<int>(tgt_tile.second) - static_cast<int>(src_tile.second);

            if (c_diff > 0) {
                if (in_tgt_col) {
                    pin = pin | ftxui::align_right;
                } else if (in_src_col) {
                    pin = ftxui::hbox({ pin, ftxui::filler() });
                }
            } else {
                if (in_src_col) {
                    pin = pin | ftxui::align_right;
                } else if (in_tgt_col) {
                    pin = ftxui::hbox({ pin, ftxui::filler() });
                }
            }
        }

        bool in_src_row = (r == src_tile.first);
        bool in_tgt_row = (r == tgt_tile.first);
        bool is_vertical_swap = is_animating && (src_tile.first != tgt_tile.first);

        if (is_vertical_swap) {
            int r_diff = static_cast<int>(tgt_tile.first) - static_cast<int>(src_tile.first);

            if (r_diff > 0) {
                if (in_tgt_row) {
                    pin = ftxui::vbox({ ftxui::filler(), pin });
                } else if (in_src_row) {
                    pin = ftxui::vbox({ pin, ftxui::filler() });
                }
            } else {
                if (in_src_row) {
                    pin = ftxui::vbox({ ftxui::filler(), pin });
                } else if (in_tgt_row) {
                    pin = ftxui::vbox({ pin, ftxui::filler() });
                }
            }
        }

        if (is_animating) {
            bool is_src = (r == src_tile.first && c == src_tile.second);
            bool is_tgt = (r == tgt_tile.first && c == tgt_tile.second);

            if (is_src || is_tgt) {
                int r_diff = static_cast<int>(tgt_tile.first) - static_cast<int>(src_tile.first);
                int c_diff = static_cast<int>(tgt_tile.second) - static_cast<int>(src_tile.second);
                int shift = anim_frame;

                if (is_tgt) {
                    r_diff = -r_diff;
                    c_diff = -c_diff;
                }

                if (c_diff != 0) {
                    if (c_diff > 0) {
                        pin = ftxui::hbox({
                            ftxui::text(std::string(shift, ' ')),
                            mtty::make_pin_anyway(board_state[r][c]),
                            ftxui::text(std::string(4 - shift, ' '))
                        });
                    } else {
                        pin = ftxui::hbox({
                            ftxui::text(std::string(4 - shift, ' ')),
                            mtty::make_pin_anyway(board_state[r][c]),
                            ftxui::text(std::string(shift, ' '))
                        });
                    }
                }
                else if (r_diff != 0) {
                    if (r_diff > 0) {
                        pin = ftxui::vbox({
                            mtty::vertical_spacer(shift),
                            mtty::make_pin_anyway(board_state[r][c]),
                            mtty::vertical_spacer(4 - shift)
                        });
                    } else {
                        pin = ftxui::vbox({
                            mtty::vertical_spacer(4 - shift),
                            mtty::make_pin_anyway(board_state[r][c]),
                            mtty::vertical_spacer(shift)
                        });
                    }
                }
                return pin | ftxui::bgcolor(ftxui::Color::DarkBlue);
            }
        }

        if (swap_back_flash_ > 0 &&
            ((r == src_tile.first && c == src_tile.second) ||
             (r == tgt_tile.first && c == tgt_tile.second))) {
            return pin | ftxui::bgcolor(ftxui::Color::Cyan);
        }

        if (is_activated) {
            pin = pin | ftxui::bgcolor(ftxui::Color::Red) | ftxui::color(ftxui::Color::White);
        } else if (is_focused) {
            pin = pin | ftxui::bgcolor(ftxui::Color::GrayDark);
        }
        return pin;
    }

    bool isAnimating() const {
        return is_animating;
    }

    bool isEliminating() const {
        return is_eliminating_;
    }

    bool isVerticalSwap() const {
        return is_animating && (src_tile.first != tgt_tile.first);
    }

    bool isHorizontalSwap() const {
        return is_animating && (src_tile.second != tgt_tile.second);
    }

    void setTimeBar(TimeBar* tb) { time_bar_ = tb; }

private:
    static constexpr int elim_hold_ticks_ = 7;

    struct EliminatingPin {
        size_t row, col;
        int delay;
    };

    void startElimination(bool from_swap = false) {
        auto matched = mtty::algo::check_match(board_state);
        if (matched.empty()) {
            if (from_swap && auto_swap_back_) {
                std::swap(board_state[src_tile.first][src_tile.second],
                          board_state[tgt_tile.first][tgt_tile.second]);
                swap_back_flash_ = 4;
                QLOG_INFO("No match, swapping back ({},{}) <-> ({},{})",
                          src_tile.first, src_tile.second,
                          tgt_tile.first, tgt_tile.second);
            }
            return;
        }

        QLOG_INFO("Found {} matched tiles, eliminating...", matched.size());

        is_eliminating_ = true;
        elim_global_frame_ = 0;
        eliminating_pins_.reserve(matched.size());

        for (auto& [r, c] : matched) {
            eliminating_pins_.push_back({r, c, 0});
        }
    }

    void finishElimination() {
        for (auto& pin : eliminating_pins_) {
            board_state[pin.row][pin.col] = generate_cb_(pin.row, pin.col);
        }
        eliminating_pins_.clear();
        startElimination();
    }

    static int defaultGenerate(size_t, size_t) {
        static std::mt19937 rng{std::random_device{}()};
        static std::uniform_int_distribution<int> dist(0, 5);
        return dist(rng);
    }

    bool is_animating;
    int anim_frame;
    bool is_fading_back;
    std::pair<size_t, size_t> src_tile;
    std::pair<size_t, size_t> tgt_tile;

    std::chrono::milliseconds frame_duration;
    std::chrono::steady_clock::time_point last_frame_time;
    std::vector<std::vector<int>> board_state;
    GenerateCallback generate_cb_;

    bool is_eliminating_;
    int elim_global_frame_;
    std::vector<EliminatingPin> eliminating_pins_;
    bool auto_swap_back_;
    int swap_back_flash_ = 0;
    TimeBar* time_bar_ = nullptr;
};
