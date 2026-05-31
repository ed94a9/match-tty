#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <match-tty/assets/components.h>
#include <match-tty/assets/mj-8pins.h>
#include <match-tty/assets/common.h>
#include <match-tty/game/GameBoardState.h>
#include <match-tty/game/TimeBar.h>
#include <match-tty/utils/Logger.h>
#include <lyra/lyra.hpp>
#include <string>
#include <memory>


struct cli_options
{
    std::size_t frame_dur_ms = 0;
    bool auto_swap_back = false;
    int game_time_secs = 60;
};

enum class cli_parse_res: std::uint8_t
{
    happy,
    help,
    error
};

auto parse_cli( int argc, char** argv ) -> std::pair<cli_parse_res, cli_options> {
    auto options{ cli_options{} };

    auto&[frame_dur_ms, auto_swap_back, game_time_secs] = options;

    bool show_help = false;
    bool disable_auto_swap_back = false;

    auto cli = lyra::cli()
        | lyra::opt( frame_dur_ms, "frame_dur_ms" )["-f"]["--frame-dur-ms"]("How long a frame is in ms. [Dev]")
        | lyra::opt( disable_auto_swap_back )["--disable-auto-swap-back"]("If one swap does not trigger a match, automatically swap back. [Dev]")
        | lyra::opt( game_time_secs, "dur" )["-d"]["--dur"]("Time limit in seconds.")
        | lyra::help( show_help )
    ;

    auto result = cli.parse( { argc, argv } );
    if ( !result ) {
    	std::cerr << "Error in command line: " << result.message() << std::endl;
        return { cli_parse_res::error, {} };
    }
    if ( show_help ) {
        std::cout << cli << std::endl;
        return { cli_parse_res::help, {} };
    }

    // Clearer logic negating this flag
    auto_swap_back = !disable_auto_swap_back;
    return { cli_parse_res::happy, options };
}

int main(int argc, char** argv )
{
    mtty::initLogger("match-tty.log");
    QLOG_INFO("--- match-tty started ---");

    auto [ parse_result, options ] = parse_cli(argc, argv);
    if (parse_result == cli_parse_res::error) return 1;
    if (parse_result == cli_parse_res::help) return 0;

    auto screen = ftxui::ScreenInteractive::TerminalOutput();

    // 1. Instantiation
    GameBoardState game(options.frame_dur_ms, options.auto_swap_back);
    auto time_bar = std::make_unique<TimeBar>();
    game.setTimeBar(time_bar.get());
    time_bar->start(options.game_time_secs, screen);

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

    auto render_next_frame = [&game, &game_grid, &screen] {
        game.UpdateAnimationTimeline(screen);
        return game_grid->Render();
    };

    auto main_layout = ftxui::Renderer(game_grid, [&] () -> ftxui::Element {
        const auto& to_render = render_next_frame();

        // --- grid area ---
        size_t fil_rows_cnt = game.isVerticalSwap() && game.isAnimating() ? 1 : 5;
        ftxui::Element grid_area = ftxui::vbox({
            ftxui::text("--- MATCH-TTY GRID ---") | ftxui::hcenter,
            ftxui::separator(),
            mtty::vertical_spacer(fil_rows_cnt),
            to_render | ftxui::hcenter,
            mtty::vertical_spacer(fil_rows_cnt),
        });

        return ftxui::vbox({
            ftxui::hbox({
                ftxui::filler(),
                grid_area,
                time_bar->Render(GameBoardState::max_rows * 3),
                ftxui::filler(),
            }),
            time_bar->isOver()
                ? ftxui::text("  GAME OVER  ") | ftxui::bold
                    | ftxui::color(ftxui::Color::RedLight) | ftxui::hcenter
                : ftxui::text(""),
        }) | ftxui::border;
    });

    screen.Loop(main_layout);

    time_bar->stop();
    return 0;
}
