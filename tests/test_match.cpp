#include <match-tty/algo/match.h>
#include <catch2/catch_all.hpp>
#include <vector>
#include <utility>

// Helper to check if a specific coordinate exists in our results vector
bool contains_coord(const std::vector<std::pair<size_t, size_t>>& results, size_t r, size_t c) {
    auto it = std::find(results.begin(), results.end(), std::make_pair(r, c));
    return it != results.end();
}

// Helper to create a clean empty grid initialized with unique values (no matches)
std::vector<std::vector<mtty::id_type>> create_clean_grid(size_t rows, size_t cols) {
    std::vector<std::vector<mtty::id_type>> grid(rows, std::vector<mtty::id_type>(cols));
    mtty::id_type counter = 1;
    for (size_t r = 0; r < rows; ++r) {
        for (size_t c = 0; c < cols; ++c) {
            // Guarantee no 3 matching elements are adjacent anywhere by incrementing sequentially
            grid[r][c] = counter++;
        }
    }
    return grid;
}

// ============================================================================
// HAPPY-PATH TEST CASES (Matches should be found perfectly)
// ============================================================================

TEST_CASE("Happy-Path: Horizontal matches of various lengths", "[match_algo]") {
    auto grid = create_clean_grid(5, 5);

    SECTION("Simple horizontal match-3") {
        grid[1][1] = 99; grid[1][2] = 99; grid[1][3] = 99; // Row 1: match of 3

        auto results = mtty::algo::check_match(grid);

        REQUIRE(results.size() == 3);
        CHECK(contains_coord(results, 1, 1));
        CHECK(contains_coord(results, 1, 2));
        CHECK(contains_coord(results, 1, 3));
    }

    SECTION("Horizontal match-4") {
        grid[3][0] = 77; grid[3][1] = 77; grid[3][2] = 77; grid[3][3] = 77;

        auto results = mtty::algo::check_match(grid);

        REQUIRE(results.size() == 4);
        CHECK(contains_coord(results, 3, 0));
        CHECK(contains_coord(results, 3, 3));
    }

    SECTION("Multiple separate horizontal matches in different rows") {
        grid[0][0] = 88; grid[0][1] = 88; grid[0][2] = 88;
        grid[4][2] = 99; grid[4][3] = 99; grid[4][4] = 99;

        auto results = mtty::algo::check_match(grid);

        REQUIRE(results.size() == 6);
        CHECK(contains_coord(results, 0, 0));
        CHECK(contains_coord(results, 4, 4));
    }
}

TEST_CASE("Happy-Path: Vertical matches and complex intersections", "[match_algo]") {
    auto grid = create_clean_grid(5, 5);

    SECTION("Simple vertical match-3") {
        grid[1][2] = 55; grid[2][2] = 55; grid[3][2] = 55;

        auto results = mtty::algo::check_match(grid);

        REQUIRE(results.size() == 3);
        CHECK(contains_coord(results, 1, 2));
        CHECK(contains_coord(results, 2, 2));
        CHECK(contains_coord(results, 3, 2));
    }

    SECTION("Intersecting L-Shape / T-Shape (Deduplication Check)") {
        // Create an intersecting T shape crossing at coordinate {2, 2}
        // Horizontal components
        grid[2][1] = 10; grid[2][2] = 10; grid[2][3] = 10;
        // Vertical components
        grid[1][2] = 10;                  grid[3][2] = 10;

        auto results = mtty::algo::check_match(grid);

        // Total matched blocks = 5 elements.
        // If deduplication fails, size would mistakenly be 6.
        REQUIRE(results.size() == 5);
        CHECK(contains_coord(results, 2, 2)); // The exact crossing junction
        CHECK(contains_coord(results, 2, 1));
        CHECK(contains_coord(results, 1, 2));
    }
}

// ============================================================================
// UNHAPPY-PATH & EDGE-CASE TEST CASES (No matches or unexpected inputs)
// ============================================================================

TEST_CASE("Unhappy-Path: Board state variants with zero matches", "[match_algo]") {

    SECTION("Completely clean board") {
        auto grid = create_clean_grid(7, 19); // Match-tty default dimensions
        auto results = mtty::algo::check_match(grid);

        CHECK(results.empty());
    }

    SECTION("Two matching adjacent elements only (Match-2 is not a match)") {
        auto grid = create_clean_grid(4, 4);
        grid[0][0] = 5; grid[0][1] = 5; // Horizontal pair
        grid[2][3] = 9; grid[3][3] = 9; // Vertical pair

        auto results = mtty::algo::check_match(grid);
        CHECK(results.empty());
    }

    SECTION("Split duplicates (3 items on a line but separated)") {
        auto grid = create_clean_grid(5, 5);
        grid[0][0] = 42; grid[0][1] = 42;
        grid[0][2] = 99; // The blocker element breaking alignment sequence!
        grid[0][3] = 42;

        auto results = mtty::algo::check_match(grid);
        CHECK(results.empty());
    }
}

TEST_CASE("Unhappy-Path: Structure boundaries and structural layout constraints", "[match_algo]") {

    SECTION("Completely empty dimensions vector matrix") {
        std::vector<std::vector<mtty::id_type>> grid;
        auto results = mtty::algo::check_match(grid);

        CHECK(results.empty());
    }

    SECTION("Empty sub-columns array representation row") {
        std::vector<std::vector<mtty::id_type>> grid = { {}, {}, {} };
        auto results = mtty::algo::check_match(grid);

        CHECK(results.empty());
    }

    SECTION("Under-sized grid window map (Smaller than 3x3)") {
        // It's physically impossible to match 3 elements on a 2x2 grid map
        std::vector<std::vector<mtty::id_type>> grid = {
            {7, 7},
            {7, 1}
        };
        auto results = mtty::algo::check_match(grid);

        CHECK(results.empty());
    }
}
