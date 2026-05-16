#pragma once

#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/dom/elements.hpp>
#include <vector>
#include <concepts>

namespace mtty {
namespace algo {

// A custom component representing an interactive 2D grid
class GridComponent : public ftxui::ComponentBase {
public:
    template <typename cb_t>
        requires requires (cb_t C) { { C(1U, 2U, false, false) } -> std::same_as<ftxui::Element>; }
    GridComponent(size_t rows, size_t cols, cb_t&& renderer_callback)
        : rows_(rows), cols_(cols), selected_row_(0), selected_col_(0)
    {
        // Re-render the grid elements when state changes
        Add(ftxui::Renderer([this, callback = std::forward<cb_t>(renderer_callback)]() {
            std::vector<std::vector<ftxui::Element>> grid_matrix(rows_, std::vector<ftxui::Element>{});

            for (size_t i = 0; i < rows_; ++i) {
                grid_matrix[i].reserve(cols_);
                for (size_t j = 0; j < cols_; ++j) {
                    bool is_focused = (i == selected_row_ && j == selected_col_);
                    bool is_activated = false; // You can expand this to track clicked/swapping pins

                    grid_matrix[i].push_back(callback(i, j, is_focused, is_activated));
                }
            }
            return ftxui::gridbox(std::move(grid_matrix));
        }));
    }

    // Capture keyboard inputs for grid navigation
    bool OnEvent(ftxui::Event event) override {
        if (event == ftxui::Event::ArrowUp || event == ftxui::Event::Character('w')) {
            if (selected_row_ > 0) { --selected_row_; return true; }
        }
        if (event == ftxui::Event::ArrowDown || event == ftxui::Event::Character('s')) {
            if (selected_row_ + 1 < rows_) { ++selected_row_; return true; }
        }
        if (event == ftxui::Event::ArrowLeft || event == ftxui::Event::Character('a')) {
            if (selected_col_ > 0) { --selected_col_; return true; }
        }
        if (event == ftxui::Event::ArrowRight || event == ftxui::Event::Character('d')) {
            if (selected_col_ + 1 < cols_) { ++selected_col_; return true; }
        }
        if (event == ftxui::Event::Return || event == ftxui::Event::Character(' ')) {
            // Handle pin selection/activation logic here
            return true;
        }

        return ComponentBase::OnEvent(event);
    }

    // Ensure the component handles focus correctly
    bool Focusable() const override { return true; }

    std::pair<size_t, size_t> GetSelectedPosition() const {
        return {selected_row_, selected_col_};
    }

private:
    size_t rows_;
    size_t cols_;
    size_t selected_row_;
    size_t selected_col_;
};

// Factory helper to construct the component seamlessly
template <typename cb_t>
inline ftxui::Component make_interactive_grid(size_t rows, size_t cols, cb_t&& callback) {
    return ftxui::Make<GridComponent>(rows, cols, std::forward<cb_t>(callback));
}

} // namespace algo
} // namespace mtty
