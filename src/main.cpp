#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <match-tty/assets/components.h>
#include <match-tty/assets/mj-8pins.h>
#include <vector>
#include <numeric>

int main() {
    auto screen = ftxui::ScreenInteractive::TerminalOutput();

    const size_t max_rows = 7;
    const size_t max_cols = 19;

    // Initialize your grid matrix state
    std::vector<std::vector<int>> board_state(max_rows, std::vector<int>(max_cols, 0));
    for (size_t r = 0; r < max_rows; ++r) {
        for (size_t c = 0; c < max_cols; ++c) {
            board_state[r][c] = (r * c) % 8; // Fill with your initial pin types
        }
    }

    // Dynamic renderer that queries our board state matrix
    auto pin_renderer = [&](size_t r, size_t c, bool is_focused, bool is_activated) {
        // Query the live, volatile matrix state
        ftxui::Element pin = mtty::make_pin_anyway(board_state[r][c]);

        if (is_activated) {
            // Selected for swapping: Flash Red/White to show lock-on state
            pin = pin | ftxui::bgcolor(ftxui::Color::Red) | ftxui::color(ftxui::Color::White);
        } else if (is_focused) {
            // Normal hover focus
            pin = pin | ftxui::bgcolor(ftxui::Color::GrayDark);
        }
        return pin;
    };

    // Callback execution whenever the user completes a swap motion
    auto swap_handler = [&](std::pair<size_t, size_t> source, std::pair<size_t, size_t> target) {
        // Modify the actual data state in memory
        std::swap(board_state[source.first][source.second], board_state[target.first][target.second]);

        // Match checking and puzzle checking logic goes right here later!
    };

    // Construct the component with both renderer and state handler hooks
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
