#pragma once

#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/dom/elements.hpp>
#include <vector>
#include <functional>
#include <utility>
#include <memory>
#include <match-tty/modifiers/clip.h>

namespace mtty {

// --- Merger System ---

class Merger {
public:
    virtual ~Merger() = default;
    virtual ftxui::Element make(const ftxui::Element& ele1, 
                                 const ftxui::Element& ele2, 
                                 size_t ele1_pixel_cnt, 
                                 size_t ele2_pixel_cnt) const = 0;
};

class VerticalStackMerger : public Merger {
public:
    ftxui::Element make(const ftxui::Element& ele1, 
                         const ftxui::Element& ele2, 
                         size_t ele1_pixel_cnt, 
                         size_t ele2_pixel_cnt) const override {
        // Use the Clip modifier to slice the elements.
        // ele1 is the falling pin, ele2 is the pin below it.
        // We assume both pins are 5 rows high.
        
        // For ele1, we keep the top 'ele1_pixel_cnt' rows.
        // For ele2, we keep the bottom 'ele2_pixel_cnt' rows.
        
        auto clipped1 = ele1 | mtty::Clip(0, ele1_pixel_cnt, 0, 5);
        auto clipped2 = ele2 | mtty::Clip(5 - ele2_pixel_cnt, 5, 0, 5);

        return ftxui::vbox({std::move(clipped1), std::move(clipped2)});
    }
};

// --- Original Content ---

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
        if (event == ftxui::Event::Return || event == ftxui::Event::Character(' ')) {
            if (is_swapping_) {
                is_swapping_ = false;
            } else {
                is_swapping_ = true;
                swap_row_ = selected_row_;
                swap_col_ = selected_col_;
            }
            return true;
        }

        int delta_row = 0;
        int delta_col = 0;

        if (event == ftxui::Event::ArrowUp    || event == ftxui::Event::Character('w')) delta_row = -1;
        if (event == ftxui::Event::ArrowDown  || event == ftxui::Event::Character('s')) delta_row = 1;
        if (event == ftxui::Event::ArrowLeft  || event == ftxui::Event::Character('a')) delta_col = -1;
        if (event == ftxui::Event::ArrowRight || event == ftxui::Event::Character('d')) delta_col = 1;

        if (delta_row != 0 || delta_col != 0) {
            int target_row = static_cast<int>(selected_row_) + delta_row;
            int target_col = static_cast<int>(selected_col_) + delta_col;

            if (target_row >= 0 && target_row < static_cast<int>(rows_) &&
                target_col >= 0 && target_col < static_cast<int>(cols_)) {

                if (is_swapping_) {
                    if (on_swap_cb_) {
                        on_swap_cb_({swap_row_, swap_col_}, {static_cast<size_t>(target_row), static_cast<size_t>(target_col)});
                    }
                    selected_row_ = target_row;
                    selected_col_ = target_col;
                    is_swapping_ = false;
                } else {
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
