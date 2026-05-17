#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <match-tty/assets/components.h>
#include <match-tty/assets/mj-8pins.h>
#include <match-tty/assets/common.h>
#include <match-tty/game/GameBoardState.h>
#include <vector>
#include <chrono>
#include <algorithm>

int main(int argc, char** argv )
{
    std::size_t frame_dur_ms = std::atoi(argv[1]);
    auto screen = ftxui::ScreenInteractive::TerminalOutput();

    // 1. Instantiation
    GameBoardState game(frame_dur_ms);

    // 2. Clean, non-cluttered callback registration hook setup
    auto game_grid = mtty::algo::make_interactive_grid(
        GameBoardState::max_rows,
        GameBoardState::max_cols,
        // Direct passthrough renderer lambda
        [&](size_t r, size_t c, bool focused, bool activated) {
            return game.RenderPin(r, c, focused, activated);
        },
        // Direct passthrough swap handler lambda
        [&](std::pair<size_t, size_t> src, std::pair<size_t, size_t> tgt) {
            game.TriggerSwap(src, tgt, screen);
        }
    );

    auto main_layout = ftxui::Renderer(game_grid, [&] () -> ftxui::Element {
        game.UpdateAnimationTimeline(screen);

        if (not game.isVerticalSwap()) {
            return ftxui::vbox({
                ftxui::text("--- MATCH-TTY GRID ---") | ftxui::hcenter,
                ftxui::separator(),
                mtty::vertical_spacer(5),
                game_grid->Render() | ftxui::hcenter,
                mtty::vertical_spacer(5),
            }) | ftxui::border;
        }

        size_t fil_rows_cnt = game.isAnimating() ? 1 : 5;
        return ftxui::vbox({
            ftxui::text("--- MATCH-TTY GRID ---") | ftxui::hcenter,
            ftxui::separator(),
            mtty::vertical_spacer(fil_rows_cnt),
            game_grid->Render() | ftxui::hcenter,
            mtty::vertical_spacer(fil_rows_cnt),
        }) | ftxui::border;
    });

    screen.Loop(main_layout);
    return 0;
}
