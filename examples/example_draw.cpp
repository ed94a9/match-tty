#include <iostream>
#include <match-tty/assets/mj-8pins.h>
#include <match-tty/algo/scene.h>

int main() {
    auto pin2 = mtty::make_pin<2>();
    auto pin3 = mtty::make_pin<3>();
    auto pin4 = mtty::make_pin<4>();
    auto pin5 = mtty::make_pin<5>();
    auto pin6 = mtty::make_pin<6>();
    auto pin9 = mtty::make_pin<9>();

    // // Display them
    // auto document = ftxui::vbox({
    //     ftxui::hbox({pin2, ftxui::text(" "), pin3, ftxui::text(" "), pin4}) | ftxui::center,
    //     ftxui::separatorEmpty(),
    //     ftxui::hbox({pin5, ftxui::text(" "), pin6, ftxui::text(" "), pin9}) | ftxui::center
    // }) | ftxui::center;

    auto document = mtty::algo::make_grid(
        // { mtty::make_pin<2>(), mtty::make_pin<3>(), mtty::make_pin<4>() },
        // { mtty::make_pin<5>(), mtty::make_pin<6>(), mtty::make_pin<9>() }
        8, 30, [](size_t i, size_t j) -> ftxui::Element {
            size_t mod = ( i + j ) % 9;
            if (mod == 0) return mtty::make_pin<9>();
            if (mod == 1) return mtty::make_pin<9>();
            if (mod == 2) return mtty::make_pin<2>();
            if (mod == 3) return mtty::make_pin<3>();
            if (mod == 4) return mtty::make_pin<4>();
            if (mod == 5) return mtty::make_pin<5>();
            if (mod == 6) return mtty::make_pin<6>();
            if (mod == 7) return mtty::make_pin<2>();
            return mtty::make_pin<3>();
        }
    );

    auto screen = ftxui::Screen::Create(ftxui::Dimension::Fit(document));
    ftxui::Render(screen, document);
    std::cout << screen.ToString() << std::endl;

    return 0;
}
