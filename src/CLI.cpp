#include "CLI.h"

#include <iostream>
#include <lyra/lyra.hpp>

auto parse_cli(int argc, char** argv) -> std::pair<cli_parse_res, cli_options>
{
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
