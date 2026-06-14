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
#include <sstream>

enum class GameState { WELCOME, PLAYING, GAME_OVER };

struct cli_options
{
    std::size_t rows = 7;
    std::size_t cols = 19;
    std::size_t frame_dur_ms = 0;
    bool auto_swap_back = false;
    int game_time_secs = 60;
    int time_gain = 1;
    int penalty_secs = 5;
    std::string log_file;
};

enum class cli_parse_res: std::uint8_t { happy, help, error };

auto parse_cli( int argc, char** argv ) -> std::pair<cli_parse_res, cli_options> {
    auto options{ cli_options{} };
    auto&[rows, cols, frame_dur_ms, auto_swap_back, game_time_secs, time_gain, penalty_secs, log_file] = options;

    bool show_help = false;
    bool disable_auto_swap_back = false;

    auto cli = lyra::cli()
        | lyra::opt( rows, "rows" )["--rows"]("Number of grid rows.")
        | lyra::opt( cols, "cols" )["--cols"]("Number of grid columns.")
        | lyra::opt( frame_dur_ms, "frame_dur_ms" )["-f"]["--frame-dur-ms"]("How long a frame is in ms. [Dev]")
        | lyra::opt( disable_auto_swap_back )["--disable-auto-swap-back"]("If one swap does not trigger a match, automatically swap back. [Dev]")
        | lyra::opt( game_time_secs, "dur" )["-d"]["--dur"]("Time limit in seconds.")
        | lyra::opt( time_gain, "time_gain" )["--time-gain"]("Multiplier for time gained per elimination batch. [Dev]")
        | lyra::opt( penalty_secs, "penalty_secs" )["--penalty"]("Seconds deducted for 3 swaps in 1s.")
        | lyra::opt( log_file, "path" )["--log"]("Enable logging to the given file path.")
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

    auto_swap_back = !disable_auto_swap_back;
    return { cli_parse_res::happy, options };
}

static const auto welcome_banner = R"(
  ___ ___       ___ ___                     ___ ___       ___ ___     
     /  /\         /  /\      ___ ___          /  /\         /  /\    
    /  /::|       /  /::\        /__/\        /  /::\       /  /:/    
   /  /:|:|      /  /:/\:\       \  \:\      /  /:/\:\     /  /:/     
  /  /:/|:|__   /  /::\ \:\       \__\:\    /  /:/  \:\   /  /::\ ___ 
 /__/:/_|::::\ /__/:/\:\_\:\      /  /::\  /__/:/ \  \:\ /__/:/\:\  /\
 \__\/  /~~/:/ \__\/  \:\/:/     /  /:/\:\ \  \:\  \__\/ \__\/  \:\/:/
       /  /:/       \__\::/     /  /:/__\/  \  \:\            \__\::/ 
      /  /:/        /  /:/     /__/:/        \  \:\           /  /:/  
     /__/:/        /__/:/      \__\/          \  \:\         /__/:/   
     \__\/         \__\/                       \__\/         \__\/    
                                          
  ___ ___       ___ ___       ___ __      
     /__/\         /__/\         |  |\    
     \  \:\        \  \:\        |  |:|   
      \__\:\        \__\:\       |  |:|   
      /  /::\       /  /::\      |__|:|__ 
     /  /:/\:\     /  /:/\:\     /  /::::\
    /  /:/__\/    /  /:/__\/    /  /:/~~~~
   /__/:/        /__/:/        /__/:/     
   \__\/         \__\/         \__\/      
)";

static const auto gameover_banner_line1 = R"(
   _________    __  _________
  / ____/   |  /  |/  / ____/
 / / __/ /| | / /|_/ / __/   
/ /_/ / ___ |/ /  / / /___   
\____/_/  |_/_/  /_/_____/   
)";

static const auto gameover_banner_line2 = R"(
   ____ _    ____________ 
  / __ \ |  / / ____/ __ \
 / / / / | / / __/ / /_/ /
/ /_/ /| |/ / /___/ _, _/ 
\____/ |___/_____/_/ |_|  
)";

ftxui::Element renderBanner(const std::string& art, ftxui::Color color) {
    std::istringstream stream(art);
    std::string line;
    ftxui::Elements lines;
    while (std::getline(stream, line)) {
        auto not_space = line.find_first_not_of(" \t");
        if (not_space == std::string::npos)
            continue;
        lines.push_back(ftxui::text(line) | ftxui::hcenter);
    }
    return ftxui::vbox(std::move(lines)) | ftxui::color(color);
}

ftxui::Element renderButton(const std::string& label, bool selected) {
    auto elem = ftxui::text(selected ? "▶ " + label + " ◀" : "  " + label + "  ")
        | ftxui::hcenter;
    if (selected)
        elem = elem | ftxui::bold | ftxui::color(ftxui::Color::GreenLight)
             | ftxui::bgcolor(ftxui::Color::GrayDark);
    else
        elem = elem | ftxui::dim;
    return elem;
}

class MainComponent : public ftxui::ComponentBase {
    ftxui::ScreenInteractive& screen_;
    cli_options options_;
    GameState state_{GameState::WELCOME};

    std::unique_ptr<GameBoardState> game_;
    std::unique_ptr<TimeBar> time_bar_;
    std::unique_ptr<ScoreBar> score_bar_;
    ftxui::Component game_grid_;
    bool was_over_ = false;
    int welcome_sel_ = 0;
    int gameover_sel_ = 0;

    void startGame() {
        gameover_sel_ = 0;
        time_bar_.reset();
        game_ = std::make_unique<GameBoardState>(
            options_.rows, options_.cols, options_.frame_dur_ms,
            options_.auto_swap_back,
            GameBoardState::defaultGenerate, options_.time_gain,
            options_.penalty_secs);
        time_bar_ = std::make_unique<TimeBar>();
        game_->setTimeBar(time_bar_.get());
        time_bar_->start(options_.game_time_secs, screen_);
        score_bar_ = std::make_unique<ScoreBar>(options_.cols);
        game_->setScoreBar(score_bar_.get());

        if (!game_grid_) {
            game_grid_ = mtty::algo::make_interactive_grid(
                game_->rows(), game_->cols(),
                [this](size_t r, size_t c, bool focused, bool a) {
                    return game_->RenderPin(r, c, focused, a);
                },
                [this](std::pair<size_t, size_t> src, std::pair<size_t, size_t> tgt) {
                    game_->TriggerSwap(src, tgt, screen_);
                });
            Add(game_grid_);
        }
        was_over_ = false;
        state_ = GameState::PLAYING;
    }

    ftxui::Element renderWelcome() {
        auto banner = renderBanner(welcome_banner, ftxui::Color::YellowLight);
        auto buttons = ftxui::hbox({
            ftxui::filler(),
            renderButton("Start", welcome_sel_ == 0),
            ftxui::text("  "),
            renderButton("Quit", welcome_sel_ == 1),
            ftxui::filler(),
        });
        return ftxui::vbox({
            ftxui::vbox({
                banner,
                ftxui::separator(),
                buttons,
            }) | ftxui::vcenter,
        }) | ftxui::border;
    }

    ftxui::Element renderPlaying() {
        if (time_bar_->getRemainingTime() <= 0 && !was_over_) {
            was_over_ = true;
            time_bar_->stop();
            gameover_sel_ = 0;
            state_ = GameState::GAME_OVER;
            return renderGameOver();
        }

        auto now = std::chrono::steady_clock::now();
        game_->UpdateAnimationTimeline(screen_);
        auto grid_elem = game_grid_->Render();
        game_->TryInsertNextFrame(screen_);

        size_t fil_rows_cnt = game_->isVerticalSwap() && game_->isAnimating() ? 1 : 5;
        ftxui::Element grid_and_bar = ftxui::vbox({
            grid_elem,
            score_bar_->Render(),
            ftxui::text("🏆 Score: " + std::to_string(score_bar_->getScore()))
                | ftxui::hcenter,
        });

        ftxui::Element grid_area = ftxui::vbox({
            ftxui::text("🎮 --- MATCH-TTY --- 🎮") | ftxui::hcenter,
            ftxui::separator(),
            mtty::vertical_spacer(fil_rows_cnt),
            ftxui::hbox({
                ftxui::filler(),
                grid_and_bar,
                ftxui::filler(),
            }),
            mtty::vertical_spacer(fil_rows_cnt),
        });

        return ftxui::vbox({
            ftxui::hbox({
                ftxui::filler(),
                grid_area,
                time_bar_->Render(static_cast<int>(game_->rows() * 3)),
                ftxui::filler(),
            }),
            [&]() -> ftxui::Element {
                std::string alert = game_->getAlert();
                if (!alert.empty())
                    return ftxui::text("  " + alert + "  ") | ftxui::bold
                        | ftxui::color(ftxui::Color::YellowLight) | ftxui::hcenter;
                return ftxui::text("");
            }(),
        }) | ftxui::border;
    }

    ftxui::Element renderGameOver() {
        std::string final_score = std::to_string(score_bar_ ? score_bar_->getScore() : 0);
        return ftxui::vbox({
            ftxui::vbox({
                renderBanner(gameover_banner_line1, ftxui::Color::RedLight),
                renderBanner(gameover_banner_line2, ftxui::Color::RedLight),
                ftxui::separator(),
                ftxui::text("🏆 Final Score: " + final_score) | ftxui::hcenter | ftxui::bold,
                ftxui::hbox({
                    ftxui::filler(),
                    renderButton("Retry", gameover_sel_ == 0),
                    ftxui::text("  "),
                    renderButton("Quit", gameover_sel_ == 1),
                    ftxui::filler(),
                }),
            }) | ftxui::vcenter,
        }) | ftxui::border;
    }

public:
    MainComponent(ftxui::ScreenInteractive& screen, cli_options opts)
        : screen_(screen), options_(std::move(opts)) { welcome_sel_ = 0; }

    ftxui::Element OnRender() override {
        switch (state_) {
        case GameState::WELCOME:  return renderWelcome();
        case GameState::PLAYING:  return renderPlaying();
        case GameState::GAME_OVER: return renderGameOver();
        }
        return ftxui::text("");
    }

    bool OnEvent(ftxui::Event e) override {
        if (state_ == GameState::WELCOME) {
            if (e == ftxui::Event::ArrowLeft || e == ftxui::Event::Character('h')) {
                welcome_sel_ = (welcome_sel_ == 0) ? 1 : 0;
                return true;
            }
            if (e == ftxui::Event::ArrowRight || e == ftxui::Event::Character('l')) {
                welcome_sel_ = (welcome_sel_ == 0) ? 1 : 0;
                return true;
            }
            if (e == ftxui::Event::Return) {
                if (welcome_sel_ == 0) startGame();
                else screen_.Exit();
                return true;
            }
            return true;
        }
        if (state_ == GameState::GAME_OVER) {
            if (e == ftxui::Event::ArrowLeft || e == ftxui::Event::Character('h')) {
                gameover_sel_ = (gameover_sel_ == 0) ? 1 : 0;
                return true;
            }
            if (e == ftxui::Event::ArrowRight || e == ftxui::Event::Character('l')) {
                gameover_sel_ = (gameover_sel_ == 0) ? 1 : 0;
                return true;
            }
            if (e == ftxui::Event::Return) {
                if (gameover_sel_ == 0) startGame();
                else screen_.Exit();
                return true;
            }
            return true;
        }
        return ComponentBase::OnEvent(e);
    }
};

int main(int argc, char** argv )
{
    auto [ parse_result, options ] = parse_cli(argc, argv);
    if (parse_result == cli_parse_res::error) return 1;
    if (parse_result == cli_parse_res::help) return 0;

    if (!options.log_file.empty()) {
        mtty::initLogger(options.log_file);
        QLOG_INFO("--- match-tty started ---");
    }

    auto screen = ftxui::ScreenInteractive::TerminalOutput();
    auto component = ftxui::Make<MainComponent>(screen, options);
    screen.Loop(component);

    return 0;
}
