#pragma once

#include <string>
#include <ftxui/dom/elements.hpp>

class ScoreBar
{
public:
    static constexpr int max_per_bar = 100;
    static constexpr int bar_width = 95; // matches grid body width (19 cols × 5 chars)

    void addScore(int count) {
        score_ += count;
        current_bar_score_ += count;
        while (current_bar_score_ >= max_per_bar) {
            current_bar_score_ -= max_per_bar;
            ++level_;
        }
    }

    int getScore() const { return score_; }

    std::string labelText() const {
        return "Score: " + std::to_string(score_) + " ";
    }

    ftxui::Element RenderLabel() const {
        return ftxui::text(labelText()) | ftxui::color(ftxui::Color::White);
    }

    ftxui::Element RenderBar() const {
        int filled = (current_bar_score_ * bar_width) / max_per_bar;
        int empty = bar_width - filled;
        auto color = barColor();
        return ftxui::hbox({
            ftxui::text(std::string(static_cast<size_t>(filled), ' ')) | ftxui::bgcolor(color),
            ftxui::text(std::string(static_cast<size_t>(empty), ' ')) | ftxui::bgcolor(ftxui::Color::GrayDark),
        });
    }

    ftxui::Element RenderGridPad() const {
        return ftxui::text(std::string(labelText().size(), ' '));
    }

    ftxui::Element Render() const {
        return ftxui::hbox({RenderLabel(), RenderBar()});
    }

private:
    int score_ = 0;
    int current_bar_score_ = 0;
    int level_ = 0;

    static ftxui::Color barColorForLevel(int lvl) {
        const ftxui::Color shades[] = {
            ftxui::Color::LightSkyBlue1,
            ftxui::Color::DeepSkyBlue1,
            ftxui::Color::Blue,
            ftxui::Color::Plum1,
            ftxui::Color::MediumPurple,
            ftxui::Color::DarkMagenta,
        };
        return shades[static_cast<size_t>(lvl) % (sizeof(shades) / sizeof(shades[0]))];
    }

    ftxui::Color barColor() const {
        return barColorForLevel(level_);
    }
};
