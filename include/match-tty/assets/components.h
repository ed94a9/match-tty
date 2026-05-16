#pragma once

#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/dom/elements.hpp>
#include <vector>
#include <functional>
#include <utility>

namespace mtty {
namespace algo {

class GridComponent : public ftxui::ComponentBase {
public:
    using SwapCallback = std::function<void(std::pair<size_t, size_t>, std::pair<size_t, size_t>)>;

    template <typename cb_t>
    GridComponent(size_t rows, size_t cols, cb_t&& renderer_callback, SwapCallback on_swap)
        : rows_(rows), cols_(cols), selected_row_(0), selected_col_(0),
          is_swapping_(false), swap_row_(0), swap_col_(0), on_swap_cb_(std::move(on_swap))
    {
        Add(ftxui::Renderer([this, callback = std::forward<cb_t>(renderer_callback)]() {
            std::vector<std::vector<ftxui::Element>> grid_matrix(rows_, std::vector<ftxui::Element>{});

            for (size_t i = 0; i < rows_; ++i) {
                grid_matrix[i].reserve(cols_);
                for (size_t j = 0; j < cols_; ++j) {
                    bool is_focused = (i == selected_row_ && j == selected_col_);
                    bool is_activated = (is_swapping_ && i == swap_row_ && j == swap_col_);

                    grid_matrix[i].push_back(callback(i, j, is_focused, is_activated));
                }
            }
            return ftxui::gridbox(std::move(grid_matrix));
        }));
    }

    bool OnEvent(ftxui::Event event) override {
        // Step 1: Detect Mode Shifting via Space/Return
        if (event == ftxui::Event::Return || event == ftxui::Event::Character(' ')) {
            if (is_swapping_) {
                // If already in swap mode, hitting space again cancels it
                is_swapping_ = false;
            } else {
                is_swapping_ = true;
                swap_row_ = selected_row_;
                swap_col_ = selected_col_;
            }
            return true;
        }

        // Step 2: Handle Movement / Action Execution
        int delta_row = 0;
        int delta_col = 0;

        if (event == ftxui::Event::ArrowUp    || event == ftxui::Event::Character('w')) delta_row = -1;
        if (event == ftxui::Event::ArrowDown  || event == ftxui::Event::Character('s')) delta_row = 1;
        if (event == ftxui::Event::ArrowLeft  || event == ftxui::Event::Character('a')) delta_col = -1;
        if (event == ftxui::Event::ArrowRight || event == ftxui::Event::Character('d')) delta_col = 1;

        if (delta_row != 0 || delta_col != 0) {
            // Calculate target coordinates
            int target_row = static_cast<int>(selected_row_) + delta_row;
            int target_col = static_cast<int>(selected_col_) + delta_col;

            // Bounds protection
            if (target_row >= 0 && target_row < static_cast<int>(rows_) &&
                target_col >= 0 && target_col < static_cast<int>(cols_)) {

                if (is_swapping_) {
                    // ACTION STATE: Execute the actual logical data modification!
                    if (on_swap_cb_) {
                        on_swap_cb_({swap_row_, swap_col_}, {static_cast<size_t>(target_row), static_cast<size_t>(target_col)});
                    }
                    // Move cursor to destination and reset mode
                    selected_row_ = target_row;
                    selected_col_ = target_col;
                    is_swapping_ = false;
                } else {
                    // NORMAL STATE: Just update cursor position
                    selected_row_ = target_row;
                    selected_col_ = target_col;
                }
                return true;
            }
        }
        return ComponentBase::OnEvent(event);
    }

    bool Focusable() const override { return true; }

private:
    size_t rows_, cols_;
    size_t selected_row_, selected_col_;

    bool is_swapping_;
    size_t swap_row_, swap_col_;
    SwapCallback on_swap_cb_;
};

template <typename cb_t>
inline ftxui::Component make_interactive_grid(size_t rows, size_t cols, cb_t&& callback, GridComponent::SwapCallback on_swap) {
    return ftxui::Make<GridComponent>(rows, cols, std::forward<cb_t>(callback), std::move(on_swap));
}

} // namespace algo
} // namespace mtty
