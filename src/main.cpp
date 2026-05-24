#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <match-tty/assets/components.h>
#include <match-tty/assets/mj-8pins.h>
#include <match-tty/assets/common.h>
#include <match-tty/game/GameBoardState.h>
#include <match-tty/utils/Logger.h>
#include <vector>
#include <chrono>
#include <algorithm>
#include <string>

int main(int argc, char** argv )
{
    mtty::initLogger("match-tty.log");
    QLOG_INFO("--- match-tty started ---");

    std::size_t frame_dur_ms = 0;
    bool auto_swap_back = false;
    int game_time_secs = 60;

    for (int i = 1; i < argc; ++i) {
        std::string arg(argv[i]);
        if (arg == "--auto-swap-back") {
            auto_swap_back = true;
        } else if (arg == "--time" && i + 1 < argc) {
            game_time_secs = std::atoi(argv[++i]);
        } else {
            frame_dur_ms = std::atoi(arg.c_str());
        }
    }

    auto screen = ftxui::ScreenInteractive::TerminalOutput();

    // 1. Instantiation
    GameBoardState game(frame_dur_ms, auto_swap_back);

    game.startTimer(game_time_secs, screen);

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

        // --- vertical timer bar ---
        int remaining = game.getTimeRemaining();
        int total = game.getTotalTime();
        int bar_h = GameBoardState::max_rows * 3;
        int filled_cnt = total > 0 ? std::min((remaining * bar_h) / total, bar_h) : 0;
        int empty_cnt = bar_h - filled_cnt;

        ftxui::Color bar_color = remaining <= 10 ? ftxui::Color::RedLight
                           : remaining <= 20 ? ftxui::Color::YellowLight
                           : ftxui::Color::GreenLight;

        std::vector<ftxui::Element> segs;
        segs.reserve(bar_h);
        for (int i = 0; i < empty_cnt; ++i)
            segs.push_back(ftxui::text("  ") | ftxui::bgcolor(ftxui::Color::GrayDark));
        for (int i = 0; i < filled_cnt; ++i)
            segs.push_back(ftxui::text("  ") | ftxui::bgcolor(bar_color));

        ftxui::Element timer_col = ftxui::vbox({
            ftxui::filler(),
            ftxui::vbox({
                ftxui::text(" " + std::to_string(remaining) + "s ")
                    | ftxui::color(ftxui::Color::White) | ftxui::hcenter,
                ftxui::vbox(std::move(segs)),
                game.isGameOver()
                    ? ftxui::text("OVER") | ftxui::bold
                        | ftxui::color(ftxui::Color::RedLight) | ftxui::hcenter
                    : ftxui::text(""),
            }),
            ftxui::filler(),
        });

        // --- grid area ---
        size_t fil_rows_cnt = game.isVerticalSwap() && game.isAnimating() ? 1 : 5;
        ftxui::Element grid_area = ftxui::vbox({
            ftxui::text("--- MATCH-TTY GRID ---") | ftxui::hcenter,
            ftxui::separator(),
            mtty::vertical_spacer(fil_rows_cnt),
            game_grid->Render() | ftxui::hcenter,
            mtty::vertical_spacer(fil_rows_cnt),
        });

        return ftxui::vbox({
            ftxui::hbox({
                ftxui::filler(),
                grid_area,
                timer_col,
                ftxui::filler(),
            }),
            game.isGameOver()
                ? ftxui::text("  GAME OVER  ") | ftxui::bold
                    | ftxui::color(ftxui::Color::RedLight) | ftxui::hcenter
                : ftxui::text(""),
        }) | ftxui::border;
    });

    screen.Loop(main_layout);

    game.stopTimer();
    return 0;
}
