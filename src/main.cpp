#include "../include/SonicUI.hpp"
#include <SDL3/SDL_mixer.h>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <iostream>
int main(int argc, char **argv) {

  if (argc < 2) {
    std::cout << "Usage: " << argv[0] << " [Audio_Dir]\n";
    return -1;
  }

  Sonic::SonicUIOptions options;
  options.freq = MIX_DEFAULT_FREQUENCY;
  options.channels = MIX_DEFAULT_CHANNELS;
  options.format = MIX_DEFAULT_FORMAT;
  options.new_path = argv[1];
  auto scr = ftxui::ScreenInteractive::Fullscreen();
  auto sonic = ftxui::Make<Sonic::SonicUI>(options);

  scr.Loop(sonic);

  return 0;
}
