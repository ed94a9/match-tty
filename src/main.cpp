#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <match-tty/assets/components.h>
#include <match-tty/assets/mj-8pins.h>

int main() {
    auto screen = ftxui::ScreenInteractive::TerminalOutput();

    // Dummy match-3 pin visualization callback
    auto pin_renderer = [](size_t r, size_t c, bool is_focused, bool is_activated) {
        // ftxui::Element pin = ftxui::text(" ◉ ") | ftxui::color(ftxui::Color::Blue);
        ftxui::Element pin = mtty::make_pin_anyway( r * c );

        if (is_focused) {
            // Apply visual highlight if the cursor is currently over this pin
            pin = pin | ftxui::bgcolor(ftxui::Color::GrayDark);
        } else {
            pin = pin;
        }
        return pin;
    };


    // Build the grid component
    auto game_grid = mtty::algo::make_interactive_grid(8, 8, pin_renderer);

    // Standard main UI wrapper
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
