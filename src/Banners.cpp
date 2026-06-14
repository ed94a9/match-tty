#include <match-tty/Banners.h>

#include <sstream>

const std::string welcome_banner = R"(
  ___ ___       ___ ___                     ___ ___       ___ ___     
     /  /\         /  /\      ___ ___          /  /\         /  /\    
    /  /::|       /  /::\        /__/\        /  /::\       /  /:/    
   /  /:|:|      /  /:/\:\       \  \:\      /  /:/\:\     /  /:/     
  /  /:/|:|__   /  /::\ \:\       \__\:\    /  /:/  \:\   /  /::\ ___ 
 /__/:/_|::::\ /__/:/\:\_\:\      /  /::\  /__/:/ \  \:\ /__/:/\:\  /\
 \__\/  /~~/:/ \__\/  \:\/:/     /  /:/\:\ \  \:\  \__\/ \__\/  \:\/:/
       /  /:/       \__\::/     /  /:/__\/  \  \:\            \__\::/ 
      /  /:/        /  /:/     /__/:/        \  \:\           /  /:/  
     /__/:/        /__/:/      \__\/          \  \:\         /__/:/   
     \__\/         \__\/                       \__\/         \__\/    
                                          
  ___ ___       ___ ___       ___ __      
     /__/\         /__/\         |  |\    
     \  \:\        \  \:\        |  |:|   
      \__\:\        \__\:\       |  |:|   
      /  /::\       /  /::\      |__|:|__ 
     /  /:/\:\     /  /:/\:\     /  /::::\
    /  /:/__\/    /  /:/__\/    /  /:/~~~~
   /__/:/        /__/:/        /__/:/     
   \__\/         \__\/         \__\/      
)";

const std::string gameover_banner_line1 = R"(
   _________    __  _________
  / ____/   |  /  |/  / ____/
 / / __/ /| | / /|_/ / __/   
/ /_/ / ___ |/ /  / / /___   
\____/_/  |_/_/  /_/_____/   
)";

const std::string gameover_banner_line2 = R"(
   ____ _    ____________ 
  / __ \ |  / / ____/ __ \
 / / / / | / / __/ / /_/ /
/ /_/ /| |/ / /___/ _, _/ 
\____/ |___/_____/_/ |_|  
)";

ftxui::Element renderBanner(const std::string& art, ftxui::Color color)
{
    std::istringstream stream(art);
    std::string line;
    ftxui::Elements lines;
    while (std::getline(stream, line)) {
        auto not_space = line.find_first_not_of(" \t");
        if (not_space == std::string::npos)
            continue;
        lines.push_back(ftxui::text(line) | ftxui::hcenter);
    }
    return ftxui::vbox(std::move(lines)) | ftxui::color(color);
}

ftxui::Element renderButton(const std::string& label, bool selected)
{
    auto elem = ftxui::text(selected ? "▶ " + label + " ◀" : "  " + label + "  ")
        | ftxui::hcenter;
    if (selected)
        elem = elem | ftxui::bold | ftxui::color(ftxui::Color::GreenLight)
             | ftxui::bgcolor(ftxui::Color::GrayDark);
    else
        elem = elem | ftxui::dim;
    return elem;
}
