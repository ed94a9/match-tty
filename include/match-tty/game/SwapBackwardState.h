#pragma once

#include "AnimState.h"
#include "SwapUtils.h"

class GameBoardState;

struct SwapBackwardState final : AnimState {
    GameBoardState& g;
    int frame = 4;
    explicit SwapBackwardState(GameBoardState& g) : g(g) {}

    AnimType type() const override { return AnimType::SwapBackward; }

    bool advance() override {
        if (--frame > 0) return true;
        return false;
    }

    ftxui::Element renderPin(size_t r, size_t c) const override {
        auto [sr, sc] = g.srcTile();
        auto [tr, tc] = g.tgtTile();
        bool is_src = (r == g.srcTile().first && c == g.srcTile().second);
        bool is_tgt = (r == g.tgtTile().first && c == g.tgtTile().second);
        return renderSwapPin(g.tileAt(r, c), is_src, is_tgt,
                             r, c, sr, tr, sc, tc, frame);
    }
};
