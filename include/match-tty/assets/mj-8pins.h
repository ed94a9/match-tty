#pragma once

#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <iostream>
#include <vector>
#include <type_traits>

namespace mtty {

namespace detail {

inline ftxui::Element create_mh_pins(std::vector<int> mask, ftxui::Color color_val, int rows = 3, int cols = 3) {
    ftxui::Elements lines;
    for (int i = 0; i < rows; ++i) {
        ftxui::Elements line;
        for (int j = 0; j < cols; ++j) {
            if (mask[i * cols + j]) {
                // Force width to exactly 1 character
                line.push_back(ftxui::text("●") | ftxui::color(color_val) | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 1));
            } else {
                // Empty cell exactly 1 character wide
                line.push_back(ftxui::text(" ") | size(ftxui::WIDTH, ftxui::EQUAL, 1));
            }
        }
        lines.push_back(ftxui::hbox(std::move(line)));
    }
    return vbox(std::move(lines)) | ftxui::border;
}

} // namespace detail

inline auto make_pin2() {
    return detail::create_mh_pins(
        {1, 0, 0,
         0, 0, 0,
         0, 0, 1}, ftxui::Color::Yellow
    );
}

inline auto make_pin3() {
    return detail::create_mh_pins(
        {1, 0, 0,
         0, 1, 0,
         0, 0, 1}, ftxui::Color::Yellow
    );
}

inline auto make_pin4() {
    return detail::create_mh_pins(
        {1, 0, 1,
         0, 0, 0,
         1, 0, 1}, ftxui::Color::Green
    );
}

inline auto make_pin5() {
    return detail::create_mh_pins(
        {1, 0, 1,
         0, 1, 0,
         1, 0, 1}, ftxui::Color::Green
    );
}

inline auto make_pin6() {
    return detail::create_mh_pins(
        {1, 0, 1,
         1, 0, 1,
         1, 0, 1}, ftxui::Color::Red
    );
}

inline auto make_pin9() {
    return detail::create_mh_pins(
        {1, 1, 1,
         1, 1, 1,
         1, 1, 1}, ftxui::Color::Red
    );
}

template <size_t cnt>
    requires ( cnt <=9 && cnt != 8 && cnt != 7 && cnt != 1 )
auto make_pin() {
    if constexpr (cnt == 2){
        return make_pin2();
    } else if constexpr (cnt == 3) {
        return make_pin3();
    } else if constexpr (cnt == 4) {
        return make_pin4();
    } else if constexpr (cnt == 5) {
        return make_pin5();
    } else if constexpr (cnt == 6) {
        return make_pin6();
    } else if constexpr (cnt == 9) {
        return make_pin9();
    }
}


} // namespace mtty
