#include <match-tty/game/GridGenerator.h>
#include <random>

std::vector<std::vector<int>> RandomGridGenerator::generate(size_t rows, size_t cols) {
    static std::mt19937 rng{std::random_device{}()};
    static std::uniform_int_distribution<int> dist(1, 6);
    std::vector<std::vector<int>> grid(rows, std::vector<int>(cols));
    for (size_t r = 0; r < rows; ++r)
        for (size_t c = 0; c < cols; ++c)
            grid[r][c] = dist(rng);
    return grid;
}

std::vector<std::vector<int>> MatchFreeGridGenerator::generate(size_t rows, size_t cols) {
    static std::mt19937 rng{std::random_device{}()};
    std::vector<std::vector<int>> grid(rows, std::vector<int>(cols));
    for (size_t r = 0; r < rows; ++r) {
        for (size_t c = 0; c < cols; ++c) {
            int val;
            do {
                static std::uniform_int_distribution<int> dist(1, 6);
                val = dist(rng);
            } while (
                (c >= 2 && grid[r][c - 1] == val && grid[r][c - 2] == val) ||
                (r >= 2 && grid[r - 1][c] == val && grid[r - 2][c] == val)
            );
            grid[r][c] = val;
        }
    }
    return grid;
}
