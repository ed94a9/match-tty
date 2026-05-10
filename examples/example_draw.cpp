#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <iostream>
#include <vector>

using namespace ftxui;

// Helper to create a strict 3-column or 2-column tile
Element MahjongTile(std::vector<int> mask, Color color_val, int rows = 3, int cols = 3) {
    Elements lines;
    for (int i = 0; i < rows; ++i) {
        Elements line;
        for (int j = 0; j < cols; ++j) {
            if (mask[i * cols + j]) {
                // Force width to exactly 1 character
                line.push_back(text("●") | color(color_val) | size(WIDTH, EQUAL, 1));
            } else {
                // Empty cell exactly 1 character wide
                line.push_back(text(" ") | size(WIDTH, EQUAL, 1));
            }
        }
        lines.push_back(hbox(std::move(line)));
    }
    return vbox(std::move(lines)) | border;
}

int main() {
    // 2-Pin (3x3 grid, dots at [0,0] and [2,2])
    auto pin2 = MahjongTile({1, 0, 0, 
                             0, 0, 0, 
                             0, 0, 1}, Color::Yellow);

    // 3-Pin (3x3 grid, diagonal)
    auto pin3 = MahjongTile({1, 0, 0, 
                             0, 1, 0, 
                             0, 0, 1}, Color::Yellow);

    // 4-Pin (3x3 grid, diagonal)
    auto pin4 = MahjongTile({1, 0, 1, 
                             0, 0, 0, 
                             1, 0, 1}, Color::Green);

    // 5-Pin (3x3 grid, X shape)
    auto pin5 = MahjongTile({1, 0, 1, 
                             0, 1, 0, 
                             1, 0, 1}, Color::Red);

    // 6-Pin (3x3 grid, two columns)
    auto pin6 = MahjongTile({1, 0, 1, 
                             1, 0, 1, 
                             1, 0, 1}, Color::Green);

    // 9-Pin (Full 3x3)
    auto pin9 = MahjongTile({1, 1, 1, 
                             1, 1, 1, 
                             1, 1, 1}, Color::Red);

    // Display them
    auto document = vbox({
        hbox({pin2, text(" "), pin3, text(" "), pin4}) | center,
        separatorEmpty(),
        hbox({pin5, text(" "), pin6, text(" "), pin9}) | center
    }) | center;

    auto screen = Screen::Create(Dimension::Fit(document));
    Render(screen, document);
    std::cout << screen.ToString() << std::endl;

    return 0;
}
