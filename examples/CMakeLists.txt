add_executable( example1 example1.cpp )
target_link_libraries( example1 PRIVATE
  ftxui::screen
  ftxui::dom
  ftxui::component)
add_executable( example_draw example_draw.cpp )

target_link_libraries( example_draw PRIVATE
  ftxui::screen
  ftxui::dom
  ftxui::component
  internal-include)
