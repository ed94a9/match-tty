#pragma once

#include <ftxui/dom/elements.hpp>
#include <string>
#include <match-tty/assets/mj-8pins.h>
#include <match-tty/assets/common.h>

inline ftxui::Element renderSwapPin(int val, bool is_src, bool is_tgt,
                                     size_t r, size_t c,
                                     int sr, int tr, int sc, int tc,
                                     int frame) {
    bool h_swap = (sc != tc);

    ftxui::Element pin = mtty::make_pin_anyway(val);

    if (h_swap) {
        if (tc > sc) {
            if (c == static_cast<size_t>(tc)) pin = pin | ftxui::align_right;
            else if (c == static_cast<size_t>(sc)) pin = ftxui::hbox({ pin, ftxui::filler() });
        } else {
            if (c == static_cast<size_t>(sc)) pin = pin | ftxui::align_right;
            else if (c == static_cast<size_t>(tc)) pin = ftxui::hbox({ pin, ftxui::filler() });
        }
    } else {
        if (tr > sr) {
            if (r == static_cast<size_t>(tr)) pin = ftxui::vbox({ ftxui::filler(), pin });
            else if (r == static_cast<size_t>(sr)) pin = ftxui::vbox({ pin, ftxui::filler() });
        } else {
            if (r == static_cast<size_t>(sr)) pin = ftxui::vbox({ ftxui::filler(), pin });
            else if (r == static_cast<size_t>(tr)) pin = ftxui::vbox({ pin, ftxui::filler() });
        }
    }

    if (is_src || is_tgt) {
        int r_diff = tr - sr;
        int c_diff = tc - sc;
        int shift = frame;

        if (is_tgt) { r_diff = -r_diff; c_diff = -c_diff; }

        if (c_diff != 0) {
            pin = c_diff > 0
                ? ftxui::hbox({ ftxui::text(std::string(shift, ' ')),
                                mtty::make_pin_anyway(val),
                                ftxui::text(std::string(4 - shift, ' ')) })
                : ftxui::hbox({ ftxui::text(std::string(4 - shift, ' ')),
                                mtty::make_pin_anyway(val),
                                ftxui::text(std::string(shift, ' ')) });
        } else if (r_diff != 0) {
            pin = r_diff > 0
                ? ftxui::vbox({ mtty::vertical_spacer(shift),
                                mtty::make_pin_anyway(val),
                                mtty::vertical_spacer(4 - shift) })
                : ftxui::vbox({ mtty::vertical_spacer(4 - shift),
                                mtty::make_pin_anyway(val),
                                mtty::vertical_spacer(shift) });
        }
        return pin | ftxui::bgcolor(ftxui::Color::DarkBlue);
    }

    return pin;
}
