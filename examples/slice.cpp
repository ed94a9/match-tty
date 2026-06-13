#include <fmt/format.h>
#include <match-tty/modifiers/clip.h>
#include <match-tty/assets/mj-8pins.h>

int main( int argc, char** argv ) {
    size_t r_start = 0;
    size_t r_end = 2;
    size_t c_start = 0;
    size_t c_end = 5;

    if (argc == 1) {
        fmt::println("[main] Got (default) clipping params(r_start, r_end, c_start, c_end): ({}, {}, {}, {})",
                    r_start, r_end, c_start, c_end);
    } else if ( argc == 5 ) {
        int int_r_start = std::atoi( argv[1] );
        int int_r_end = std::atoi( argv[2] );
        int int_c_start = std::atoi( argv[3] );
        int int_c_end = std::atoi( argv[4] );
        if (not (int_r_start >= 0 and int_r_end >= int_r_start and int_c_start >= 0 and int_c_end >= int_c_start) )
            throw std::invalid_argument( fmt::format("WRONG parameters!") );

        r_start = int_r_start;
        r_end = int_r_end;
        c_start = int_c_start;
        c_end = int_c_end;
        fmt::println("[main] Got (default) clipping params(r_start, r_end, c_start, c_end): ({}, {}, {}, {})",
                    r_start, r_end, c_start, c_end);
    } else {
        throw std::invalid_argument( fmt::format("WRONG Number of parameters! Got: {}, Expect: 0 or 4", argc) );
    }

    auto pin9_clipped = mtty::make_pin<5>() | mtty::Clip( r_start, r_end, c_start, c_end );

    auto screen = ftxui::Screen::Create(
      ftxui::Dimension::Fit(pin9_clipped),
      ftxui::Dimension::Fit(pin9_clipped)
    );

    // Render the document onto the screen.
    ftxui::Render(screen, pin9_clipped);

    // Print the screen to the console.
    screen.Print();
}
