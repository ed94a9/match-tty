#pragma once

#include <ftxui/dom/elements.hpp>

namespace mtty {

inline auto vertical_spacer(int height) -> ftxui::Element {
    ftxui::Elements rows( height, ftxui::text("") );
    return ftxui::vbox(std::move(rows));
};




}
