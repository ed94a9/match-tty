#include "CLI.h"
#include "MainComponent.h"

#include <ftxui/component/screen_interactive.hpp>
#include <match-tty/utils/Logger.h>

int main(int argc, char** argv)
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
