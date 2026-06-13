#pragma once

#include "AnimState.h"
#include <match-tty/assets/mj-8pins.h>
#include <match-tty/assets/common.h>
#include <match-tty/modifiers/clip.h>
#include <ftxui/dom/elements.hpp>
#include <vector>
#include <algorithm>
#include <set>

class GameBoardState;

struct FallDownState final : AnimState {
    GameBoardState& g;

    struct Move {
        int src_px;
        int tgt_px;
        size_t col;
        int value;
    };
    std::vector<Move> moves_;

    bool moved_ = false;
    std::vector<std::vector<int>> final_board_;

    int frame_ = 0;
    static constexpr int total_frames_ = 8;

    explicit FallDownState(GameBoardState& g);

    AnimType type() const override { return AnimType::FallDown; }
    bool advance() override;
    ftxui::Element renderPin(size_t r, size_t c) const override;

    const std::vector<std::vector<int>>& finalBoard() const { return final_board_; }
};
