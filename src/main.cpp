#include "../include/SonicUI.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <iostream>
int main(int argc, char **argv) {

  auto scr = ftxui::ScreenInteractive::Fullscreen();
  auto sonic = ftxui::Make<Sonic::SonicUI>();

  scr.Loop(sonic);

  return 0;
}
