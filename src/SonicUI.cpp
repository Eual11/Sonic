#include "../include/SonicUI.hpp"
#include <SDL3/SDL_mixer.h>
#include <SDL3/SDL_system.h>
#include <algorithm>
#include <atomic>
#include <filesystem>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/node.hpp>
#include <mutex>
#include <stdio.h>
#include <string>
#include <thread>

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
  m_CurrentTrack.albumIndex = 0; // setting the dummy track
                                 //
  m_CurrentTrack.queueindex = 0;
  m_PrevTrack = m_CurrentTrack;
  m_NextTrack = m_CurrentTrack;
  refresh_audio_queue = true;
  /* AudioHandlerThread.swap(swapthread); */

  refresh_audio_queue = true;
  AudioHandlerThread = std::thread(&SonicUI::AudioQueueHandler, this);
  Add(sonicui);
}

bool Sonic::SonicUI::OnEvent(ftxui::Event event) {

  // p: play/pause
  // n: next track
  // b: back track
  // left/right arrow: perhaps sicking ?
  // q: quit
  //
  /* std::lock_guard<std::mutex> lock(mtx); // locking */
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
    if (MainWindow->Focused()) {
      std::lock_guard<std::mutex> lock(mtx);
      m_PrevTrack = m_CurrentTrack;
      // updating current track
      m_CurrentTrack.albumIndex = m_LeftPanelSelected;
      m_CurrentTrack.queueindex = m_MainWindowSelected;
      m_CurrentTrack.audio = AudioQueue[m_MainWindowSelected];

      // setting next Track
      //

      int nextAudioTrackIndex = m_MainWindowSelected + 1;
      int nextTrackSelectionIndex = m_LeftPanelSelected;
      if (nextAudioTrackIndex >= (int)AudioQueue.size()) {
        nextAudioTrackIndex = 0;
        nextTrackSelectionIndex += 1;
      }
      if (nextTrackSelectionIndex >= (int)LeftPanelSelection.size())
        nextTrackSelectionIndex = 0;

      m_NextTrack.albumIndex = nextTrackSelectionIndex;
      m_NextTrack.queueindex = nextAudioTrackIndex;

      if (nextTrackSelectionIndex == m_LeftPanelSelected)
        m_NextTrack.audio = AudioQueue[m_NextTrack.queueindex];

      m_StartedPlaying = true;
      AudioPlayer.TogglePlay(m_CurrentTrack.audio);
    }
    return true;
  }

  if (event == ftxui::Event::Character("[")) {
    // seek backwards
    std::lock_guard<std::mutex> lock(mtx);
    AudioPlayer.backwardCurrentAudio();
  }
  if (event == ftxui::Event::Character("]")) {
    std::lock_guard<std::mutex> lock(mtx);
    AudioPlayer.forwardCurrentAudio();
  }
  if (event == ftxui::Event::Character('i')) {
    AudioPlayer.setVolume(AudioPlayer.VOLUME + 5);
    return true;
  }
  if (event == ftxui::Event::Character(" ")) {
    AudioPlayer.TogglePlay(m_CurrentTrack.audio);
  }

  if (event == ftxui::Event::Character('k')) {
    AudioPlayer.setVolume(AudioPlayer.VOLUME - 5);
    return true;
  }
  if (event == ftxui::Event::Character('r')) {
    // rewind
    //
    AudioPlayer.rewindCurrentAudio();
    return true;
  }

  return ftxui::ComponentBase::OnEvent(event);
}

ftxui::Element Sonic::SonicUI::Render() {

  auto bgcolor = ftxui::bgcolor(hexToRGB("#002b36"));
  auto color = ftxui::color(hexToRGB("#637c76"));
  auto guagecolor = ftxui::color(hexToRGB("#2a84c0"));
  auto nextplaycolor = ftxui::color(hexToRGB("#6676c6"));
  auto volumecolor = ftxui::color(hexToRGB("#85720c"));
  auto curplayingcolor = ftxui::color(hexToRGB("#da4282"));
  auto LeftPanelView = ftxui::vbox({ftxui::text("Artists") | ftxui::center,
                                    ftxui::text(" "), LeftPanel->Render()});
  auto MainWindowView = ftxui::vbox({ftxui::text("Tracks") | ftxui::center,
                                     ftxui::text(" "), MainWindow->Render()});
  ftxui::Element mainView =
      ftxui::hbox({LeftPanelView | ftxui::frame | ftxui::borderLight |
                       ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 30),
                   MainWindowView | ftxui::yframe | ftxui::vscroll_indicator |
                       ftxui::flex | ftxui::borderLight});

  auto volumeSlider = ftxui::Slider("Vol ", &AudioPlayer.VOLUME, 0, 100, 5);
  volumeSlider |=
      ftxui::Renderer([&](ftxui::Element e) { return e | volumecolor; });

  float duration = AudioPlayer.getCurrentAudioDuration();

  int mins = (int)(duration) / 60;

  // a stupid way to 0 pad secsonds and mins
  int secs = (int)(duration) % 60;

  std::string cur_duration; // = mins + ":" + secs;
  // possibly unsafe
  char buffer[80];
  buffer[79] = '\0';
  sprintf(buffer, "%02d:%02d", mins, secs);
  cur_duration = buffer;

  mins = (int)(m_CurrentTrack.audio.duration) / 60;
  secs = (int)(m_CurrentTrack.audio.duration) % 60;
  mins = mins < 0 ? 0 : mins;
  secs = secs < 0 ? 0 : secs;
  buffer[79] = '\0';
  sprintf(buffer, "%02d:%02d", mins, secs);

  std::string audio_duration;
  audio_duration = buffer;
  auto playbackGauge = ftxui::Slider(cur_duration, &duration, 0.0f,
                                     (float)m_CurrentTrack.audio.duration, 1);
  playbackGauge |=
      ftxui::Renderer([&](ftxui::Element e) { return e | guagecolor; });
  auto slider_width = ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 15);
  auto statusline_view =
      ftxui::vbox(
          {ftxui::hbox(
               {ftxui::text("Playing "),
                ftxui::text(m_CurrentTrack.audio.title) | curplayingcolor,
                ftxui::text("   Next "),
                ftxui::text(m_NextTrack.audio.title) | nextplaycolor}),
           ftxui::hbox(
               {volumeSlider->Render() | volumecolor | slider_width,
                ftxui::text(
                    std::to_string((int)((AudioPlayer.VOLUME / 128.0f) * 100)) +
                    "%"),
                ftxui::text("   "),
                playbackGauge->Render() | guagecolor | ftxui::flex,
                ftxui::text(audio_duration),
                ftxui::text("  Loop|Shuffle  ") | volumecolor})}) |
      ftxui::size(ftxui::HEIGHT, ftxui::EQUAL, 3) | ftxui::borderLight;

  return ftxui::vbox({mainView | ftxui::flex, statusline_view}) | bgcolor |
         color;
}

void Sonic::SonicUI::Load_Libraries(std::filesystem::path new_path) {

  // load audio trachs from libraries

  /* std::lock_guard<std::mutex> lock(mtx); */
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
  std::unique_lock<std::mutex> lock(mtx);
  LeftPanelSelection.clear();
  for (auto track : TracksList) {
    switch (m_TrackSelector) {
    case TrackSelection::ARTIST: {
      if (std::find(LeftPanelSelection.begin(), LeftPanelSelection.end(),
                    track.artist) == LeftPanelSelection.end())
        LeftPanelSelection.push_back(track.artist);

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
  lock.unlock();
  UpdateLeftPanelView();
  // update the left panel view and mainwindow view
}

void Sonic::SonicUI::UpdateAudioQueue(int index) {
  std::unique_lock<std::mutex> lock(mtx);
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

  lock.unlock();
  UpdateMainWindowView();
}
void Sonic::SonicUI::UpdateLeftPanelView(void) {

  std::lock_guard<std::mutex> lock(mtx);
  LeftPanel->DetachAllChildren();

  for (auto &selection : LeftPanelSelection) {
    LeftPanel->Add(ftxui::MenuEntry(selection, LeftPanelEntryOption));
  }
}

void Sonic::SonicUI::UpdateMainWindowView(void) {

  std::lock_guard<std::mutex> lock(mtx);
  MainWindow->DetachAllChildren();

  for (auto &track : AudioQueue) {
    MainWindow->Add(ftxui::MenuEntry(track.title, MainWindowEntryOption));
  }
}
void Sonic::SonicUI::UpdateAllSelection(void) {
  UpdataLeftPanelSelection();
  UpdateAudioQueue(m_LeftPanelSelected);
}
void Sonic::SonicUI::AudioQueueHandler(void) {

  //
  //
  //
  while (refresh_audio_queue) {
    std::unique_lock<std::mutex> lock(mtx);
    if (AudioPlayer.isAudioPlaying() || !m_StartedPlaying) {
      lock.unlock();
      using namespace std::chrono_literals;
      std::this_thread::sleep_for(1s);

    } else {
      // play next on the queue;
      //
      m_PrevTrack = m_CurrentTrack;
      // i don't even know if this good or not
      lock.unlock();
      UpdateAudioQueue(m_NextTrack.albumIndex);
      lock.lock();
      LeftPanel->SetActiveChild(LeftPanel->ChildAt(m_NextTrack.albumIndex));

      m_NextTrack.audio = AudioQueue[m_NextTrack.queueindex];
      m_CurrentTrack = m_NextTrack;
      AudioPlayer.TogglePlay(m_CurrentTrack.audio);

      // load next track
      int nextAudioTrackIndex = m_MainWindowSelected + 1;
      int nextTrackSelectionIndex = m_LeftPanelSelected;
      if (nextAudioTrackIndex >= (int)AudioQueue.size()) {
        nextAudioTrackIndex = 0;
        nextTrackSelectionIndex += 1;
      }
      if (nextTrackSelectionIndex >= (int)LeftPanelSelection.size())
        nextTrackSelectionIndex = 0;

      m_NextTrack.albumIndex = nextTrackSelectionIndex;
      m_NextTrack.queueindex = nextAudioTrackIndex;
    }
  }
}
