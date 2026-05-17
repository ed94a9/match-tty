#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <match-tty/assets/components.h>
#include <match-tty/assets/mj-8pins.h>
#include <vector>
#include <chrono>
#include <algorithm>

int main() {
    auto screen = ftxui::ScreenInteractive::TerminalOutput();

    const size_t max_rows = 7;
    const size_t max_cols = 19;

    std::vector<std::vector<int>> board_state(max_rows, std::vector<int>(max_cols, 0));
    for (size_t r = 0; r < max_rows; ++r) {
        for (size_t c = 0; c < max_cols; ++c) {
            board_state[r][c] = (r * c) % 8;
        }
    }

    // --- SINGLE THREADED ANIMATION STATE ---
    bool is_animating = false;
    int anim_frame = 0;
    bool is_fading_back = false; // Tracks if we are in Phase 1 (sliding out) or Phase 3 (retracting)
    std::pair<size_t, size_t> src_tile;
    std::pair<size_t, size_t> tgt_tile;

    // Time tracking to lock frame rate manually
    auto last_frame_time = std::chrono::steady_clock::now();
    const std::chrono::milliseconds frame_duration(30); // 30ms per frame

    auto pin_renderer = [&](size_t r, size_t c, bool is_focused, bool is_activated) {
        ftxui::Element pin = mtty::make_pin_anyway(board_state[r][c]);

        if (is_animating) {
            bool is_src = (r == src_tile.first && c == src_tile.second);
            bool is_tgt = (r == tgt_tile.first && c == tgt_tile.second);

            if (is_src || is_tgt) {
                int r_diff = static_cast<int>(tgt_tile.first) - static_cast<int>(src_tile.first);
                int c_diff = static_cast<int>(tgt_tile.second) - static_cast<int>(src_tile.second);
                int shift_spaces = anim_frame;

                if (c_diff != 0) { // HORIZONTAL
                    bool move_right = (is_src && c_diff > 0) || (is_tgt && c_diff < 0);
                    if (move_right) {
                        pin = ftxui::hbox({ftxui::text(std::string(shift_spaces, ' ')), pin});
                    } else {
                        pin = ftxui::hbox({pin, ftxui::text(std::string(shift_spaces, ' '))});
                    }
                }
                else if (r_diff != 0) { // VERTICAL
                    bool move_down = (is_src && r_diff > 0) || (is_tgt && r_diff < 0);
                    if (move_down) {
                        pin = ftxui::vbox({ftxui::text(std::string(shift_spaces, ' ')), pin});
                    } else {
                        pin = ftxui::vbox({pin, ftxui::text(std::string(shift_spaces, ' '))});
                    }
                }
                return pin | ftxui::bgcolor(ftxui::Color::DarkBlue);
            }
        }

        if (is_activated) {
            pin = pin | ftxui::bgcolor(ftxui::Color::Red) | ftxui::color(ftxui::Color::White);
        } else if (is_focused) {
            pin = pin | ftxui::bgcolor(ftxui::Color::GrayDark);
        }
        return pin;
    };

    // Triggered once at the beginning of a swap action
    auto swap_handler = [&](std::pair<size_t, size_t> source, std::pair<size_t, size_t> target) {
        if (is_animating) return;

        is_animating = true;
        anim_frame = 0;
        is_fading_back = false;
        src_tile = source;
        tgt_tile = target;
        last_frame_time = std::chrono::steady_clock::now();

        // Kickstart the single-threaded frame loop event chain
        screen.PostEvent(ftxui::Event::Custom);
    };

    auto game_grid = mtty::algo::make_interactive_grid(max_rows, max_cols, pin_renderer, swap_handler);

    // Standard main UI wrapper context
    auto main_layout = ftxui::Renderer(game_grid, [&] {
        // --- THE ENGINE TICK (Runs every time the screen redraws) ---
        if (is_animating) {
            auto now = std::chrono::steady_clock::now();

            // Only advance the frame if our 30ms target has passed
            if (now - last_frame_time >= frame_duration) {
                last_frame_time = now;

                if (!is_fading_back) {
                    // Phase 1: Sliding out
                    if (anim_frame < 4) {
                        anim_frame++;
                    } else {
                        // Phase 2: Boundary reached! Swap values in place
                        std::swap(board_state[src_tile.first][src_tile.second],
                                  board_state[tgt_tile.first][tgt_tile.second]);
                        is_fading_back = true;
                    }
                } else {
                    // Phase 3: Sliding back in
                    if (anim_frame > 0) {
                        anim_frame--;
                    } else {
                        // Animation complete!
                        is_animating = false;
                    }
                }
            }

            // If we are still animating, request another loop cycle immediately
            if (is_animating) {
                screen.PostEvent(ftxui::Event::Custom);
            }
        }

        return ftxui::vbox({
            ftxui::text("--- MATCH-TTY GRID (SINGLE-THREADED) ---") | ftxui::hcenter,
            ftxui::separator(),
            game_grid->Render() | ftxui::hcenter,
        }) | ftxui::border;
    });

    screen.Loop(main_layout);
    return 0;
}
