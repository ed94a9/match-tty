add_executable( main main.cpp Logger.cpp GameBoardState.cpp FallDownState.cpp GridGenerator.cpp )

target_link_libraries( main PRIVATE
  fmt::fmt
  ftxui::screen
  ftxui::dom
  ftxui::component
  quill::quill
  bfg::lyra
  internal-include
)
