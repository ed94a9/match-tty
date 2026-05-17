#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <match-tty/assets/components.h>
#include <match-tty/assets/mj-8pins.h>
#include <vector>
#include <chrono>
#include <algorithm>

int main(int argc, char** argv ) {

    std::size_t frame_dur_ms = std::atoi(argv[1]);
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
    bool is_fading_back = false;
    std::pair<size_t, size_t> src_tile;
    std::pair<size_t, size_t> tgt_tile;

    auto last_frame_time = std::chrono::steady_clock::now();
    // const std::chrono::milliseconds frame_duration(300);
    const std::chrono::milliseconds frame_duration(frame_dur_ms);

    auto pin_renderer = [&](size_t r, size_t c, bool is_focused, bool is_activated) -> ftxui::Element {
        ftxui::Element pin = mtty::make_pin_anyway(board_state[r][c]);

        // 1. CHOOSE BASE ALIGNMENT BASED ON ROLES IN HORIZONTAL SWAPS
        bool in_src_col = (c == src_tile.second);
        bool in_tgt_col = (c == tgt_tile.second);
        bool is_horizontal_swap = is_animating && (src_tile.second != tgt_tile.second);

        if (is_horizontal_swap) {
            int c_diff = static_cast<int>(tgt_tile.second) - static_cast<int>(src_tile.second);

            if (c_diff > 0) { // Moving Right (Source is Left, Target is Right)
                if (in_tgt_col) {
                    // All static pins in the right key column push flush to the right edge
                    pin = pin | ftxui::align_right;
                } else if (in_src_col) {
                    // All static pins in the left key column stay flush to the left edge
                    pin = ftxui::hbox({ pin, ftxui::filler() });
                }
            } else { // Moving Left (Source is Right, Target is Left)
                if (in_src_col) {
                    pin = pin | ftxui::align_right;
                } else if (in_tgt_col) {
                    pin = ftxui::hbox({ pin, ftxui::filler() });
                }
            }
        }

        // 2. CHOOSE BASE ALIGNMENT BASED ON ROLES IN VERTICAL SWAPS
        bool in_src_row = (r == src_tile.first);
        bool in_tgt_row = (r == tgt_tile.first);
        bool is_vertical_swap = is_animating && (src_tile.first != tgt_tile.first);

        if (is_vertical_swap) {
            int r_diff = static_cast<int>(tgt_tile.first) - static_cast<int>(src_tile.first);

            if (r_diff > 0) { // Moving Down (Source is Top, Target is Bottom)
                if (in_tgt_row) {
                    // All static pins in the bottom key row must push flush to the bottom edge
                    pin = ftxui::vbox( {ftxui::filler(), pin} );
                } else if (in_src_row) {
                    // All static pins in the top key row stay flush to the top edge
                    pin = ftxui::vbox( {pin, ftxui::filler()} );
                }
            } else { // Moving Up (Source is Bottom, Target is Top)
                if (in_src_row) {
                    pin = ftxui::vbox( {ftxui::filler(), pin} );
                } else if (in_tgt_row) {
                    pin = ftxui::vbox( {pin, ftxui::filler()} );
                }
            }
        }


        // 2. APPLY ANIMATION OFFSET OVERRIDES ONLY FOR THE ACTIVE SWAP TILES
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

                if (c_diff != 0) { // HORIZONTAL SWAP ANIMATION
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
                else if (r_diff != 0) { // VERTICAL SWAP ANIMATION
                    auto vertical_spacer = [](int height) {
                        ftxui::Elements rows;
                        for (int i = 0; i < height; ++i) rows.push_back(ftxui::text(""));
                        return ftxui::vbox(std::move(rows));
                    };

                    if (r_diff > 0) {
                        pin = ftxui::vbox({
                            vertical_spacer(shift),
                            mtty::make_pin_anyway(board_state[r][c]),
                            vertical_spacer(4 - shift)
                        });
                    } else {
                        pin = ftxui::vbox({
                            vertical_spacer(4 - shift),
                            mtty::make_pin_anyway(board_state[r][c]),
                            vertical_spacer(shift)
                        });
                    }
                }
                return pin | ftxui::bgcolor(ftxui::Color::DarkBlue);
            }
        }

        // 3. APPLY STANDARD FOCUS/ACTIVATION STYLES
        if (is_activated) {
            pin = pin | ftxui::bgcolor(ftxui::Color::Red) | ftxui::color(ftxui::Color::White);
        } else if (is_focused) {
            pin = pin | ftxui::bgcolor(ftxui::Color::GrayDark);
        }
        return pin;
    };

    auto swap_handler = [&](std::pair<size_t, size_t> source, std::pair<size_t, size_t> target) {
        if (is_animating) return;

        is_animating = true;
        anim_frame = 0;
        is_fading_back = false;
        src_tile = source;
        tgt_tile = target;
        last_frame_time = std::chrono::steady_clock::now();

        screen.PostEvent(ftxui::Event::Custom);
    };

    auto game_grid = mtty::algo::make_interactive_grid(max_rows, max_cols, pin_renderer, swap_handler);

    auto main_layout = ftxui::Renderer(game_grid, [&] () -> ftxui::Element {
        if (is_animating) {
            auto now = std::chrono::steady_clock::now();

            if (now - last_frame_time >= frame_duration) {
                last_frame_time = now;

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
                    }
                }
            }

            if (is_animating) {
                screen.PostEvent(ftxui::Event::Custom);
            }
        }

        // FIX 2 & 3: Cleaned up inner returns and validated brace layouts
        return ftxui::vbox({
            ftxui::text("--- MATCH-TTY GRID (SINGLE-THREADED) ---") | ftxui::hcenter,
            ftxui::separator(),
            game_grid->Render() | ftxui::hcenter,
        }) | ftxui::border;
    });

    screen.Loop(main_layout);
    return 0;
}
