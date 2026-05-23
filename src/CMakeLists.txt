add_executable( main main.cpp Logger.cpp )

target_link_libraries( main PRIVATE
  fmt::fmt
  ftxui::screen
  ftxui::dom
  ftxui::component
  quill::quill
  internal-include
)
