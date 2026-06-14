#pragma once

#include <cstdint>
#include <string>
#include <utility>

struct cli_options
{
    std::size_t rows = 7;
    std::size_t cols = 19;
    std::size_t frame_dur_ms = 10;
    bool auto_swap_back = false;
    int game_time_secs = 60;
    int time_gain = 1;
    int penalty_secs = 5;
    std::string log_file;
};

enum class cli_parse_res: std::uint8_t { happy, help, error };

auto parse_cli(int argc, char** argv) -> std::pair<cli_parse_res, cli_options>;
