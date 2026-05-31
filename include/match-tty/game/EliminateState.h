#pragma once

#include "AnimState.h"
#include <match-tty/assets/mj-8pins.h>

class GameBoardState;

struct EliminateState final : AnimState {
    GameBoardState& g;
    int frame = 0;
    explicit EliminateState(GameBoardState& g) : g(g) {}

    AnimType type() const override { return AnimType::Eliminate; }

    bool advance() override {
        ++frame;
        for (auto& pin : g.eliminatingPins())
            if ((frame - pin.delay) / g.elimHoldTicks() < 2)
                return true;
        return false;
    }

    ftxui::Element renderPin(size_t r, size_t c) const override {
        for (auto& pin : g.eliminatingPins()) {
            if (pin.row == r && pin.col == c) {
                int local = frame - pin.delay;
                if (local < 0) break;
                int visual = local / g.elimHoldTicks();
                if (visual == 0)
                    return mtty::make_pin_center_only(g.tileAt(r, c))
                         | ftxui::bgcolor(ftxui::Color::DarkBlue);
                if (visual == 1)
                    return mtty::make_center_dot()
                         | ftxui::bgcolor(ftxui::Color::DarkBlue);
                return ftxui::text(" ")
                     | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 1);
            }
        }
        return {};
    }
};
