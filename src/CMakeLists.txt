add_executable( match-tty
  main.cpp
  CLI.cpp
  Banners.cpp
  MainComponent.cpp
  Logger.cpp
  GameBoardState.cpp
  FallDownState.cpp
  GridGenerator.cpp
)

target_link_libraries( match-tty PRIVATE
  fmt::fmt
  ftxui::screen
  ftxui::dom
  ftxui::component
  quill::quill
  bfg::lyra
  internal-include
)
