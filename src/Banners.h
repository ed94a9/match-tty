#pragma once

#include <string>
#include <ftxui/dom/elements.hpp>

extern const std::string welcome_banner;
extern const std::string gameover_banner_line1;
extern const std::string gameover_banner_line2;

ftxui::Element renderBanner(const std::string& art, ftxui::Color color);
ftxui::Element renderButton(const std::string& label, bool selected);
