#pragma once

#include "AnimState.h"
#include <match-tty/assets/mj-8pins.h>

class GameBoardState;

struct FlashState final : AnimState {
    GameBoardState& g;
    int counter = 4;
    explicit FlashState(GameBoardState& g) : g(g) {}

    AnimType type() const override { return AnimType::Flash; }

    bool advance() override {
        return --counter > 0;
    }

    ftxui::Element renderPin(size_t r, size_t c) const override {
        if ((r == g.srcTile().first && c == g.srcTile().second) ||
            (r == g.tgtTile().first && c == g.tgtTile().second))
            return mtty::make_pin_anyway(g.tileAt(r, c))
                 | ftxui::bgcolor(ftxui::Color::Cyan);
        return {};
    }
};
