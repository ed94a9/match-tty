#pragma once

#include <ftxui/dom/elements.hpp>

class GameBoardState;

enum class AnimType { SwapForward, SwapBackward, Eliminate, Flash };

struct AnimState {
    virtual ~AnimState() = default;
    virtual AnimType type() const = 0;
    virtual bool advance() = 0;
    virtual ftxui::Element renderPin(size_t r, size_t c) const = 0;
};
