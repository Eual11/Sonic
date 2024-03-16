#include "../include/SonicUI.hpp"
#include <SDL3/SDL_mixer.h>
#include <SDL3/SDL_system.h>
#include <algorithm>
#include <filesystem>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/node.hpp>

namespace fs = std::filesystem;

Sonic::SonicUI::SonicUI(Sonic::SonicUIOptions options) {

  m_TrackSelector = TrackSelection::ARTIST;
  ftxui::Component left_main =
      ftxui::Container::Horizontal({LeftPanel, MainWindow});
  ftxui::Component sonicui =
      ftxui::Container::Vertical({left_main, StatusLine});

  fs::path audio_dir(options.new_path);
  if (!fs::exists(audio_dir) || !fs::is_directory(audio_dir)) {
    std::cout << "Path: " << audio_dir
              << "doesn't exist or it's not a valid dir\n";
    exit(1);
  }

  SonicAudioPlayer AudioPlayer(options.freq, options.channels, options.format);
  Load_Libraries(audio_dir);
  Add(sonicui);
}

bool Sonic::SonicUI::OnEvent(ftxui::Event event) {

  // p: play/pause
  // n: next track
  // b: back track
  // left/right arrow: perhaps sicking ?
  // q: quit
  if (LeftPanel->Focused() && event == ftxui::Event::ArrowDown) {
    UpdateAudioQueue((m_LeftPanelSelected + 1) % LeftPanelSelection.size());
  } else if (LeftPanel->Focused() && event == ftxui::Event::ArrowUp) {
    UpdateAudioQueue((m_LeftPanelSelected - 1));
  }
  if (event == ftxui::Event::ArrowLeft) {
    return true;
  }
  if (event == ftxui::Event::ArrowRight) {
    return true;
  }
  if (event == ftxui::Event::Tab) {
    if (LeftPanel->Focused()) {

      MainWindow->TakeFocus();
    } else if (MainWindow->Focused()) {
      LeftPanel->TakeFocus();
    }
    return true;
  }
  if (event == ftxui::Event::Character('p')) {
    if (MainWindow->Focused())
      AudioPlayer.TogglePlay(AudioQueue[m_MainWindowSelected]);
    return true;
  }

  return ftxui::ComponentBase::OnEvent(event);
}

ftxui::Element Sonic::SonicUI::Render() {

  ftxui::Element mainView = ftxui::hbox(
      {LeftPanel->Render() | ftxui::frame | ftxui::borderLight |
           ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 30),
       MainWindow->Render() | ftxui::yframe | ftxui::vscroll_indicator |
           ftxui::flex | ftxui::borderLight});

  ftxui::Element statusline_view =
      ftxui::hbox(
          {ftxui::text("this is empty") | ftxui::flex | ftxui::borderLight}) |
      ftxui::size(ftxui::HEIGHT, ftxui::EQUAL, 5);

  return ftxui::vbox({mainView | ftxui::flex, statusline_view});
}

void Sonic::SonicUI::Load_Libraries(std::filesystem::path new_path) {

  // load audio trachs from libraries

  for (auto dir_file : std::filesystem::directory_iterator(new_path)) {
    if (dir_file.is_regular_file() &&
        AudioPlayer.isSupported(dir_file.path().extension().string())) {

      Sonic::SonicAudio audio(fs::absolute(dir_file.path()));
      // if the audio file is a valid playable music file

      // TODO: needs optimization
      if (audio.path != "") {
        TracksList.push_back(audio);
      }
    }
  }
  UpdateAllSelection();
}
void Sonic::SonicUI::UpdataLeftPanelSelection(void) {
  LeftPanelSelection.clear();
  for (auto track : TracksList) {
    switch (m_TrackSelector) {
    case TrackSelection::ARTIST: {
      if (std::find(LeftPanelSelection.begin(), LeftPanelSelection.end(),
                    track.artist) == LeftPanelSelection.end())
        LeftPanelSelection.push_back(track.artist);
      std::cerr << track.artist << std::endl;

      break;
    }
    case TrackSelection::ALBUM: {
      /* std::cerr << "Supported ALBUM\n"; */
      if (std::find(LeftPanelSelection.begin(), LeftPanelSelection.end(),
                    track.album) == LeftPanelSelection.end())
        LeftPanelSelection.push_back(track.album);

      //        LeftPanelSelection.push_back(track.album);
      break;
    }
    case TrackSelection::PLAYLIST: {
      // unimplmented
      break;
    }
    }
  }
  UpdateLeftPanelView();
  // update the left panel view and mainwindow view
}

void Sonic::SonicUI::UpdateAudioQueue(int index) {
  index = index < 0 ? 0 : index;
  AudioQueue.clear();
  for (auto track : TracksList) {
    switch (m_TrackSelector) {
    case TrackSelection::ALBUM: {
      if (track.album == LeftPanelSelection[index]) {
        AudioQueue.push_back(track);
      }

      break;
    }
    case TrackSelection::ARTIST: {
      if (track.artist == LeftPanelSelection[index]) {
        AudioQueue.push_back(track);
        // do something
      }
      break;
    }
    case TrackSelection::PLAYLIST: {
      // unimplmented
      break;
    }
    }
  }

  UpdateMainWindowView();
}
void Sonic::SonicUI::UpdateLeftPanelView(void) {

  LeftPanel->DetachAllChildren();

  for (auto &selection : LeftPanelSelection) {
    LeftPanel->Add(ftxui::MenuEntry(selection, LeftPanelEntryOption));
  }
}

void Sonic::SonicUI::UpdateMainWindowView(void) {

  MainWindow->DetachAllChildren();

  std::cerr << "AudioQueue size: " << AudioQueue.size() << std::endl;
  for (auto &track : AudioQueue) {
    /* std::cerr << "title: " << track.title << " artist: " << track.artist */
    /* << " album; " << track.album << " path: " << track.path */
    /* << std::endl; */
    MainWindow->Add(ftxui::MenuEntry(track.title, MainWindowEntryOption));
  }
}
void Sonic::SonicUI::UpdateAllSelection(void) {
  UpdataLeftPanelSelection();
  UpdateAudioQueue(m_LeftPanelSelected);
}
