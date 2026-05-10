#include <iostream>
#include <match-tty/assets/mj-8pins.h>

int main() {
    auto pin2 = mtty::make_pin2();
    auto pin3 = mtty::make_pin3();
    auto pin4 = mtty::make_pin4();
    auto pin5 = mtty::make_pin5();
    auto pin6 = mtty::make_pin6();
    auto pin9 = mtty::make_pin9();

    // Display them
    auto document = ftxui::vbox({
        ftxui::hbox({pin2, ftxui::text(" "), pin3, ftxui::text(" "), pin4}) | ftxui::center,
        ftxui::separatorEmpty(),
        ftxui::hbox({pin5, ftxui::text(" "), pin6, ftxui::text(" "), pin9}) | ftxui::center
    }) | ftxui::center;

    auto screen = ftxui::Screen::Create(ftxui::Dimension::Fit(document));
    ftxui::Render(screen, document);
    std::cout << screen.ToString() << std::endl;

    return 0;
}
