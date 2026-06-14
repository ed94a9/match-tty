#pragma once

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <match-tty/game/GameBoardState.h>
#include <match-tty/game/ScoreBar.h>
#include <match-tty/game/TimeBar.h>
#include <memory>
#include "CLI.h"

enum class GameState { WELCOME, PLAYING, GAME_OVER };

class MainComponent : public ftxui::ComponentBase
{
public:
    MainComponent(ftxui::ScreenInteractive& screen, cli_options opts);

    ftxui::Element OnRender() override;
    bool OnEvent(ftxui::Event e) override;

private:
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

    void startGame();
    ftxui::Element renderWelcome();
    ftxui::Element renderPlaying();
    ftxui::Element renderGameOver();
};
