#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/node.hpp>
#include <ftxui/screen/screen.hpp>
#include <fmt/format.h>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <type_traits>
#include <match-tty/assets/mj-8pins.h>

namespace mtty {

    // A custom DOM Node that holds a pre-rendered grid fragment
    class SlicedNode final: public ftxui::Node {
    public:
        SlicedNode(int width, int height, std::vector<std::vector<ftxui::Pixel>> pixels)
            : width_(width), height_(height), pixels_(std::move(pixels)) {}

        // Tell FTXUI exactly how big this sliced asset footprint is
        void ComputeRequirement() override {
            requirement_.min_x = width_;
            requirement_.min_y = height_;
        }

        // Blit our captured pixels back onto the final screen target
        void Render(ftxui::Screen& screen) override {
            for (int y = 0; y < height_; ++y) {
                for (int x = 0; x < width_; ++x) {
                    int target_x = box_.x_min + x;
                    int target_y = box_.y_min + y;

                    // Boundary check within the allocated box allocation
                    if (target_x <= box_.x_max && target_y <= box_.y_max) {
                        screen.PixelAt(target_x, target_y) = pixels_[y][x];
                    }
                }
            }
        }

    private:
        int width_;
        int height_;
        std::vector<std::vector<ftxui::Pixel>> pixels_;
    };

    struct Clip {
        size_t r_start;
        size_t r_end;
        size_t c_start;
        size_t c_end;
    };

    ftxui::Element clip(const ftxui::Element& ele, size_t r_start, size_t r_end, size_t c_start, size_t c_end) {
        int target_height = static_cast<int>(r_end - r_start);
        int target_width  = static_cast<int>(c_end - c_start);

        if (target_height <= 0 || target_width <= 0) {
            return ftxui::text("");
        }

        // --- THE FIX IS HERE ---
        // 1. Ask the element how big it naturally wants to be when unconstrained
        ele->ComputeRequirement();
        int natural_width = ele->requirement().min_x;
        int natural_height = ele->requirement().min_y;

        // Fallback to safe defaults if calculation returns zero
        if (natural_width == 0) natural_width = 100;
        if (natural_height == 0) natural_height = 100;

        // 2. Create the staging screen tailored precisely to its natural size bounds
        auto staging_screen = ftxui::Screen::Create(
            ftxui::Dimension::Fixed(natural_width),
            ftxui::Dimension::Fixed(natural_height)
        );
        ftxui::Render(staging_screen, ele);
        // -----------------------

        // 3. Extract the true cell states from the grid matrix
        std::vector<std::vector<ftxui::Pixel>> captured_pixels(
            target_height, std::vector<ftxui::Pixel>(target_width)
        );

        for (int y = 0; y < target_height; ++y) {
            for (int x = 0; x < target_width; ++x) {
                int src_x = static_cast<int>(c_start) + x;
                int src_y = static_cast<int>(r_start) + y;

                if (src_x < staging_screen.dimx() && src_y < staging_screen.dimy()) {
                    captured_pixels[y][x] = staging_screen.PixelAt(src_x, src_y);
                }
            }
        }

        // 4. Package the grid into our custom node element layout pointer
        return std::make_shared<SlicedNode>(target_width, target_height, std::move(captured_pixels));
    }

    ftxui::Element operator| ( const ftxui::Element& ele, Clip clip_config ) {
        return clip( ele, clip_config.r_start, clip_config.r_end, clip_config.c_start, clip_config.c_end );
    }

}

int main() {
    auto pin9_clipped = mtty::make_pin_anyway(9) | mtty::Clip( 0, 2, 0, 5 );

    auto screen = ftxui::Screen::Create(
      ftxui::Dimension::Fit(pin9_clipped),
      ftxui::Dimension::Fit(pin9_clipped)
    );

    // Render the document onto the screen.
    ftxui::Render(screen, pin9_clipped);

    // Print the screen to the console.
    screen.Print();
}
