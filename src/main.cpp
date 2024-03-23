#include "../include/SonicUI.hpp"
#include <SDL3/SDL_mixer.h>
#include <chrono>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <iostream>
#include <string.h>
int main(int argc, char **argv) {

  std::string new_library = "."; // current working dir;
  if (argc > 2) {

    std::cerr << "ADDED PATH" << std::endl;
    if (strcmp(argv[1], "-a") == 0) {
      // add a new library

      new_library = argv[2];
    }
  }

  Sonic::SonicUIOptions options;
  options.freq = MIX_DEFAULT_FREQUENCY;
  options.channels = MIX_DEFAULT_CHANNELS;
  options.format = MIX_DEFAULT_FORMAT;
  options.new_path = new_library;
  auto scr = ftxui::ScreenInteractive::Fullscreen();
  auto sonic = ftxui::Make<Sonic::SonicUI>(options);
  sonic->setQuitFunction(scr.ExitLoopClosure());
  std::atomic<bool> refresh_ui_continue(true);
  std::thread refresh_ui([&] {
    while (refresh_ui_continue) {

      using namespace std::chrono_literals;
      std::this_thread::sleep_for(0.75s); // make this thread sleep for 0.75
                                          // secs

      // request a new frame to be rendered every 0.75 secs, this si
      // approimately 2 frames persecond
      scr.PostEvent(ftxui::Event::Custom);
    }
  });

  scr.Loop(sonic);

  refresh_ui_continue = false;
  refresh_ui.join();
  return 0;
}
