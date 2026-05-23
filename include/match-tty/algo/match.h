#include <vector>
#include <utility>
#include <set>
#include <cstdint>

namespace mtty {

using id_type = int;

namespace algo {

/**
 * Dumb algo. Bad complexity. Need to update later on
 */

inline std::vector<std::pair<std::size_t, std::size_t>> check_match(const std::vector<std::vector<id_type>>& grid) {
    if (grid.empty() || grid[0].empty()) return {};

    std::size_t max_rows = grid.size();
    std::size_t max_cols = grid[0].size();

    // Use a std::set to automatically filter out duplicate coordinates
    // from intersecting horizontal and vertical matches (e.g., T-shapes or L-shapes).
    std::set<std::pair<std::size_t, std::size_t>> matched_coordinates;

    // 1. Check Horizontal Matches (Row by Row)
    for (std::size_t r = 0; r < max_rows; ++r) {
        std::size_t match_start = 0;
        for (std::size_t c = 1; c <= max_cols; ++c) {
            // If we hit a different tile type, or reach the end of the row
            if (c == max_cols || grid[r][c] != grid[r][match_start]) {
                std::size_t match_length = c - match_start;
                if (match_length >= 3) {
                    for (std::size_t i = match_start; i < c; ++i) {
                        matched_coordinates.insert({r, i});
                    }
                }
                match_start = c; // Reset start tracker for the next potential match
            }
        }
    }

    // 2. Check Vertical Matches (Column by Column)
    for (std::size_t c = 0; c < max_cols; ++c) {
        std::size_t match_start = 0;
        for (std::size_t r = 1; r <= max_rows; ++r) {
            // If we hit a different tile type, or reach the end of the column
            if (r == max_rows || grid[r][c] != grid[match_start][c]) {
                std::size_t match_length = r - match_start;
                if (match_length >= 3) {
                    for (std::size_t i = match_start; i < r; ++i) {
                        matched_coordinates.insert({i, c});
                    }
                }
                match_start = r; // Reset start tracker for the next potential match
            }
        }
    }

    // Convert the unique coordinates set back to a vector for game state processing
    return {matched_coordinates.begin(), matched_coordinates.end()};
}

} // namespace algo

} // namespace mtty
