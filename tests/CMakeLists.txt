add_library( ftxui-interface INTERFACE )
target_link_libraries( ftxui-interface INTERFACE
  ftxui::screen
  ftxui::dom
  ftxui::component
  fmt::fmt
  Catch2::Catch2WithMain
)


add_executable( test_screen test_screen.cpp )
target_link_libraries( test_screen PRIVATE ftxui-interface fmt::fmt)

add_executable( test_dom test_dom.cpp )
target_link_libraries( test_dom PRIVATE ftxui-interface fmt::fmt)

add_executable( test_match test_match.cpp )
target_link_libraries( test_match PRIVATE ftxui-interface fmt::fmt Catch2::Catch2WithMain internal-include  )
