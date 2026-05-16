#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <match-tty/assets/components.h>
#include <match-tty/assets/mj-8pins.h>
#include <vector>
#include <thread>
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

    // --- POSITIONAL ANIMATION STATE ---
    bool is_animating = false;
    int anim_frame = 0; // 0 to 4 (5 frames total)
    std::pair<size_t, size_t> src_tile;
    std::pair<size_t, size_t> tgt_tile;

    auto pin_renderer = [&](size_t r, size_t c, bool is_focused, bool is_activated) {
        // Fetch the standard static Mahjong tile element
        ftxui::Element pin = mtty::make_pin_anyway(board_state[r][c]);

        if (is_animating) {
            bool is_src = (r == src_tile.first && c == src_tile.second);
            bool is_tgt = (r == tgt_tile.first && c == tgt_tile.second);

            if (is_src || is_tgt) {
                int r_diff = static_cast<int>(tgt_tile.first) - static_cast<int>(src_tile.first);
                int c_diff = static_cast<int>(tgt_tile.second) - static_cast<int>(src_tile.second);

                // Determine shift amount based on frame (0 to 4)
                // Frame 0: No shift | Frame 4: Shifted maximum before data swap
                int shift_spaces = anim_frame;

                if (c_diff != 0) { // HORIZONTAL SWAP
                    // If moving right, add padding to the left to push the character right
                    bool move_right = (is_src && c_diff > 0) || (is_tgt && c_diff < 0);

                    if (move_right) {
                        pin = ftxui::hbox({ftxui::text(std::string(shift_spaces, ' ')), pin});
                    } else {
                        pin = ftxui::hbox({pin, ftxui::text(std::string(shift_spaces, ' '))});
                    }
                }
                else if (r_diff != 0) { // VERTICAL SWAP
                    bool move_down = (is_src && r_diff > 0) || (is_tgt && r_diff < 0);

                    if (move_down) {
                        pin = ftxui::vbox({ftxui::text(std::string(shift_spaces, ' ')), pin});
                    } else {
                        pin = ftxui::vbox({pin, ftxui::text(std::string(shift_spaces, ' '))});
                    }
                }

                // Give the active moving pair a distinctive background color
                return pin | ftxui::bgcolor(ftxui::Color::DarkBlue);
            }
        }

        // Static rendering defaults
        if (is_activated) {
            pin = pin | ftxui::bgcolor(ftxui::Color::Red) | ftxui::color(ftxui::Color::White);
        } else if (is_focused) {
            pin = pin | ftxui::bgcolor(ftxui::Color::GrayDark);
        }
        return pin;
    };

    auto swap_handler = [&](std::pair<size_t, size_t> source, std::pair<size_t, size_t> target) {
        if (is_animating) return;

        src_tile = source;
        tgt_tile = target;

        std::thread([&screen, &board_state, &is_animating, &anim_frame, source, target]() {
            is_animating = true;

            // Phase 1: Slide out toward target cells
            for (anim_frame = 0; anim_frame < 5; ++anim_frame) {
                screen.PostEvent(ftxui::Event::Custom);
                std::this_thread::sleep_for(std::chrono::milliseconds(30)); // Snappy 30ms transitions
            }

            // Phase 2: Core State Swap (Happens instantly behind the scenes)
            std::swap(board_state[source.first][source.second], board_state[target.first][target.second]);

            // Phase 3: Slide back into alignment from the new positions
            // We swap our tracking assignments so the visual interpolation retracts seamlessly
            for (anim_frame = 4; anim_frame >= 0; --anim_frame) {
                screen.PostEvent(ftxui::Event::Custom);
                std::this_thread::sleep_for(std::chrono::milliseconds(30));
            }

            is_animating = false;
            screen.PostEvent(ftxui::Event::Custom);
        }).detach();
    };

    auto game_grid = mtty::algo::make_interactive_grid(max_rows, max_cols, pin_renderer, swap_handler);

    auto main_layout = ftxui::Renderer(game_grid, [&] {
        return ftxui::vbox({
            ftxui::text("--- MATCH-TTY GRID ---") | ftxui::hcenter,
            ftxui::separator(),
            game_grid->Render() | ftxui::hcenter,
        }) | ftxui::border;
    });

    screen.Loop(main_layout);
    return 0;
}
