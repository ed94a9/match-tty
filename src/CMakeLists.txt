add_executable( main main.cpp )

target_link_libraries( main PRIVATE
  fmt::fmt
  ftxui::screen
  ftxui::dom
  ftxui::component
  internal-include
)
