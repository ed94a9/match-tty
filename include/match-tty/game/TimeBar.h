#pragma once

#include <atomic>
#include <thread>
#include <chrono>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>

class TimeBar
{
public:
    TimeBar() = default;

    ~TimeBar() { stop(); }

    void start(int seconds, ftxui::ScreenInteractive& screen) {
        if (seconds <= 0) return;
        total_time_ = seconds;
        time_remaining_ = seconds;
        timer_running_ = true;
        timer_thread_ = std::thread([this, &screen]() {
            while (timer_running_) {
                if (time_remaining_ <= 0) {
                    game_over_ = true;
                    screen.PostEvent(ftxui::Event::Custom);
                    break;
                }
                std::this_thread::sleep_for(std::chrono::seconds(1));
                if (timer_running_) {
                    --time_remaining_;
                    screen.PostEvent(ftxui::Event::Custom);
                }
            }
        });
    }

    void stop() {
        timer_running_ = false;
        if (timer_thread_.joinable()) {
            timer_thread_.join();
        }
    }

    int getRemainingTime() const { return time_remaining_.load(); }
    int getTotalTime() const { return total_time_; }
    void addTime(int seconds) {
        if (seconds <= 0) return;
        int current = time_remaining_.load();
        int total = total_time_;
        int added = std::min(current + seconds, total);
        time_remaining_.store(added);
    }

    bool isOver() const { return game_over_.load(); }

    ftxui::Element Render(int max_height) const {
        int remaining = getRemainingTime();
        int total = getTotalTime();
        int filled = total > 0 ? std::min((remaining * max_height) / total, max_height) : 0;
        int empty = max_height - filled;

        ftxui::Color bar_color = remaining <= 10 ? ftxui::Color::RedLight
                           : remaining <= 20 ? ftxui::Color::YellowLight
                           : ftxui::Color::GreenLight;

        std::vector<ftxui::Element> segs;
        segs.reserve(max_height);
        for (int i = 0; i < empty; ++i)
            segs.push_back(ftxui::text("  ") | ftxui::bgcolor(ftxui::Color::GrayDark));
        for (int i = 0; i < filled; ++i)
            segs.push_back(ftxui::text("  ") | ftxui::bgcolor(bar_color));

        return ftxui::vbox({
            ftxui::filler(),
            ftxui::vbox({
                ftxui::text(" " + std::to_string(remaining) + "s ")
                    | ftxui::color(ftxui::Color::White) | ftxui::hcenter,
                ftxui::vbox(std::move(segs)),
                isOver()
                    ? ftxui::text("OVER") | ftxui::bold
                        | ftxui::color(ftxui::Color::RedLight) | ftxui::hcenter
                    : ftxui::text(""),
            }),
            ftxui::filler(),
        });
    }

private:
    std::atomic<int> time_remaining_{0};
    std::atomic<bool> game_over_{false};
    std::atomic<bool> timer_running_{false};
    std::thread timer_thread_;
    int total_time_ = 0;
};
