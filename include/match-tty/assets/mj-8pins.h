#pragma once

#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <fmt/format.h>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <type_traits>

namespace mtty {

namespace detail {

inline ftxui::Element make_inner_dots(std::vector<int> mask, ftxui::Color color_val) {
    ftxui::Elements lines;
    for (int i = 0; i < 3; ++i) {
        ftxui::Elements line;
        for (int j = 0; j < 3; ++j) {
            if (mask[i * 3 + j]) {
                line.push_back(ftxui::text("●") | ftxui::color(color_val) | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 1));
            } else {
                line.push_back(ftxui::text(" ") | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 1));
            }
        }
        lines.push_back(ftxui::hbox(std::move(line)));
    }
    return ftxui::vbox(std::move(lines));
}

inline ftxui::Element create_mh_pins(std::vector<int> mask, ftxui::Color color_val) {
    return make_inner_dots(std::move(mask), color_val) | ftxui::border ;
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
         1, 0, 1}, ftxui::Color::LightCoral
    );
}

inline auto make_pin9() {
    return detail::create_mh_pins(
        {1, 1, 1,
         1, 1, 1,
         1, 1, 1}, ftxui::Color::LightCoral
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

inline auto make_pin( size_t cnt ) {
    if ( not (
        cnt <= 9 && cnt != 8 && cnt != 7 && cnt != 1
    )) {
        throw std::invalid_argument(
            fmt::format("[mtty::make_pin] Only able to make pin of number <= 1 and not any of [1, 7, 8]! Got: {}", cnt));
    }
    if (cnt == 2){
        return make_pin2();
    } else if (cnt == 3) {
        return make_pin3();
    } else if (cnt == 4) {
        return make_pin4();
    } else if (cnt == 5) {
        return make_pin5();
    } else if (cnt == 6) {
        return make_pin6();
    } else if (cnt == 9) {
        return make_pin9();
    }
}

// Maps any integer to one of the 6 available visual pins {2,3,4,5,6,9}
inline auto make_pin_anyway( std::int64_t any_int ) {
    if (any_int <= 0) return make_pin<2>();
    auto idx = (any_int - 1) % 6;
    if (idx == 0) return make_pin<2>();
    if (idx == 1) return make_pin<3>();
    if (idx == 2) return make_pin<4>();
    if (idx == 3) return make_pin<5>();
    if (idx == 4) return make_pin<6>();
    return make_pin<9>();
}

// Center pixel inside a border that has moved one pixel inward (surrounding the
// inner 3×3 of the original 5×5 footprint). Used for elimination frame 0.
inline ftxui::Element make_pin_center_only(std::int64_t any_int) {
    if (any_int <= 0) any_int = 1;
    auto idx = (any_int - 1) % 6;
    static const std::vector<std::vector<int>> masks = {
        {1,0,0, 0,0,0, 0,0,1},
        {1,0,0, 0,1,0, 0,0,1},
        {1,0,1, 0,0,0, 1,0,1},
        {1,0,1, 0,1,0, 1,0,1},
        {1,0,1, 1,0,1, 1,0,1},
        {1,1,1, 1,1,1, 1,1,1},
    };
    static const std::vector<ftxui::Color> colors = {
        ftxui::Color::Yellow,
        ftxui::Color::Yellow,
        ftxui::Color::Green,
        ftxui::Color::Green,
        ftxui::Color::LightCoral,
        ftxui::Color::LightCoral,
    };
    auto& mask = masks[static_cast<size_t>(idx)];
    auto color = colors[static_cast<size_t>(idx)];

    auto center_str = std::string(mask[4] ? "●" : " ");
    auto center_el = ftxui::text(center_str) | ftxui::color(color) | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 1);

    return ftxui::vbox({
        ftxui::text("     "),
        ftxui::text(" ╭─╮ "),
        ftxui::hbox({
            ftxui::text(" │"),
            center_el,
            ftxui::text("│ "),
        }),
        ftxui::text(" ╰─╯ "),
        ftxui::text("     "),
    });
}

// Outlined circle, padded to 5×5. Used for elimination frame 1.
inline ftxui::Element make_center_dot() {
    return ftxui::hbox({
        ftxui::text("  "),
        ftxui::vbox({
            ftxui::text(" "),
            ftxui::text(" "),
            ftxui::text("○") | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 1),
            ftxui::text(" "),
            ftxui::text(" "),
        }),
        ftxui::text("  "),
    });
}

} // namespace mtty
