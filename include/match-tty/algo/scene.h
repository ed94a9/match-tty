#pragma once

#include <match-tty/assets/mj-8pins.h>
#include <vector>
#include <variant>
#include <concepts>

namespace mtty {

namespace algo {

inline ftxui::Element make_grid( const std::vector<std::vector<ftxui::Element>>& grid_contents ) {
    return ftxui::gridbox(grid_contents);
}

template <typename cb_t>
    requires requires ( cb_t C ) {{ C(1U, 2U) } -> std::same_as<ftxui::Element>;}
inline ftxui::Element make_grid( size_t rows, size_t cols, cb_t&& callback ) {
    std::vector<std::vector<ftxui::Element>> res( rows, std::vector<ftxui::Element>{} );

    size_t i = 0;
    for ( auto& row: res ) {
        row.reserve( cols );
        for ( size_t j = 0; j < cols; ++j ) {
            row.push_back( callback( i, j ) );
        }
        ++i;
    }
    return ftxui::gridbox(res);
}

} // namespace algo


} // namespace mtty
