#pragma once

#include <vector>
#include <cstddef>
#include <memory>

struct GridGenerator {
    virtual ~GridGenerator() = default;
    virtual std::vector<std::vector<int>> generate(size_t rows, size_t cols) = 0;
};

struct RandomGridGenerator : GridGenerator {
    std::vector<std::vector<int>> generate(size_t rows, size_t cols) override;
};

struct MatchFreeGridGenerator : GridGenerator {
    std::vector<std::vector<int>> generate(size_t rows, size_t cols) override;
};
