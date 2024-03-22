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
#include <stdlib.h>
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
      std::unique_lock<std::mutex> lock(mtx);
      // updating current track
      m_CurrentTrack.albumIndex = m_LeftPanelSelected;
      m_CurrentTrack.queueindex = m_MainWindowSelected;
      m_CurrentTrack.audio = AudioQueue[m_MainWindowSelected];

      // setting next Track

      m_PrevTrack = m_CurrentTrack;
      m_StartedPlaying = true;
      AudioPlayer.TogglePlay(m_CurrentTrack.audio);
      lock.unlock();
      loadNextTrack(AudioPlayer.m_Shuffle);
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
  if (event == ftxui::Event::Character('n')) {
    mtx.lock();
    mtx.unlock();
    playNextTrack();
    return true;
  }
  if (event == ftxui::Event::Character('b')) {
    // prev

    mtx.lock();
    m_CurrentTrack = m_PrevTrack;
    AudioPlayer.TogglePlay(m_CurrentTrack.audio);
    mtx.unlock();
    loadNextTrack(AudioPlayer.m_Shuffle);
    return true;
  }
  if (event == ftxui::Event::Character('l')) {
    mtx.lock();
    AudioPlayer.m_Loop = !AudioPlayer.m_Loop;
    mtx.unlock();
    return true;
  }
  if (event == ftxui::Event::Character('s')) {
    mtx.lock();
    AudioPlayer.m_Shuffle = !AudioPlayer.m_Shuffle;
    mtx.unlock();
    return true;
  }
  if (event == ftxui::Event::Character('r')) {
    // rewind
    //
    AudioPlayer.rewindCurrentAudio();
    return true;
  }
  if (event == ftxui::Event::Character('1')) {
    m_TrackSelector = TrackSelection::ARTIST;
    UpdateAllSelection();
    return true;
  }

  if (event == ftxui::Event::Character('2')) {
    m_TrackSelector = TrackSelection::ALL_TRACKS;
    UpdateAllSelection();
    return true;
  }
  if (event == ftxui::Event::Character('3')) {
    m_TrackSelector = TrackSelection::ALBUM;
    UpdateAllSelection();
    return true;
  }

  // quit
  //
  if (event == ftxui::Event::Character('q')) {
    refresh_audio_queue = false;
    AudioHandlerThread.join();
    quit();
    /* exit(0); */
  }
  return ftxui::ComponentBase::OnEvent(event);
}

ftxui::Element Sonic::SonicUI::Render() {

  /* auto bgcolor = ftxui::bgcolor(hexToRGB("#002b36")); */
  auto bgcolor = ftxui::bgcolor(hexToRGB("#19212f"));
  auto color = ftxui::color(hexToRGB("#8ea2c4"));
  auto loopDecorator = AudioPlayer.m_Loop ? ftxui::bold : ftxui::dim;
  auto shuffleDecorator = AudioPlayer.m_Shuffle ? ftxui::bold : ftxui::dim;
  auto guagecolor = ftxui::color(hexToRGB("#2a84c0"));
  auto nextplaycolor = ftxui::color(hexToRGB("#d44bec"));
  auto volumecolor = ftxui::color(hexToRGB("#85720c"));
  auto curplayingcolor = ftxui::color(hexToRGB("#74d046"));
  auto leftPanelHeader = ftxui::text(getSelectionHeader());
  auto mainWindowHeader =
      ftxui::text("Tracks"); // this may be subjected to change
  if (LeftPanel->Focused())
    leftPanelHeader |=
        ftxui::bgcolor(hexToRGB("#56B6C2")) | ftxui::color(hexToRGB("#19212f"));
  if (MainWindow->Focused())
    mainWindowHeader |=
        ftxui::bgcolor(hexToRGB("#56B6C2")) | ftxui::color(hexToRGB("#19212f"));
  auto LeftPanelView = ftxui::vbox(
      {leftPanelHeader | ftxui::center, ftxui::text(" "), LeftPanel->Render()});
  auto MainWindowView = ftxui::vbox({mainWindowHeader | ftxui::center,
                                     ftxui::text(" "), MainWindow->Render()});
  ftxui::Element mainView =
      ftxui::hbox({LeftPanelView | ftxui::yframe | ftxui::borderLight |
                       ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 30),
                   MainWindowView | ftxui::yframe | ftxui::vscroll_indicator |
                       ftxui::flex | ftxui::borderLight});

  auto volumeSlider = ftxui::Slider(" ", &AudioPlayer.VOLUME, 0, 100, 5);
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
  std::string playerState = m_StartedPlaying && !AudioPlayer.isPaused()
                                ? " Playing "
                                : " Paused ";
  std::string audio_duration;
  audio_duration = buffer;
  float progress = (float)AudioPlayer.getCurrentAudioDuration() /
                   (float)(m_CurrentTrack.audio.duration);
  auto playbackGauge = ftxui::gauge(progress);
  auto slider_width = ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 15);
  auto statusline_view =
      ftxui::vbox(
          {ftxui::hbox(
               {ftxui::text(playerState),
                ftxui::text(m_CurrentTrack.audio.title) | curplayingcolor,
                ftxui::text("   Next "),
                ftxui::text(m_NextTrack.audio.title) | nextplaycolor}),
           ftxui::hbox(
               {volumeSlider->Render() | volumecolor | slider_width,
                ftxui::text(
                    std::to_string((int)((AudioPlayer.VOLUME / 128.0f) * 100)) +
                    "%"),
                ftxui::text("  " + cur_duration + " "),
                playbackGauge | guagecolor | ftxui::flex,
                ftxui::text(audio_duration),
                ftxui::text("  Loop") | ftxui::color(hexToRGB("#ff4761")) |
                    loopDecorator,
                ftxui::separator(),
                ftxui::text("Shuffle") | ftxui::color(hexToRGB("#f7bc47")) |
                    shuffleDecorator})}) |

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

  if (m_TrackSelector != TrackSelection::ALL_TRACKS) {
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

        break;
      }
      case TrackSelection::PLAYLIST: {
        // unimplmented
        break;
      }
      }
    }
  } else {
    LeftPanelSelection.push_back("All Tracks");
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
    case TrackSelection::ALL_TRACKS: {
      AudioQueue.push_back(track);
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
    auto entry = ftxui::MenuEntry(selection, LeftPanelEntryOption);
    entry |= ftxui::Renderer([&](ftxui::Element e) {
      return ftxui::hbox(
                 ftxui::text(" 󰲸 ") | ftxui::color(hexToRGB("#008dd4")), e) |
             ftxui::bold;
    });

    LeftPanel->Add(entry);
  }
}

void Sonic::SonicUI::UpdateMainWindowView(void) {

  std::lock_guard<std::mutex> lock(mtx);
  MainWindow->DetachAllChildren();

  for (auto &track : AudioQueue) {
    auto entry = ftxui::MenuEntry(track.title, MainWindowEntryOption);
    entry |= ftxui::Renderer([&](ftxui::Element e) {
      return ftxui::hbox(
          ftxui::text("  ") | ftxui::color(hexToRGB("#009f98")), e);
    });
    MainWindow->Add(entry);
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
      m_PrevTrack = m_CurrentTrack;
      // play next on the queue;
      lock.unlock();
      playNextTrack();
    }
  }
}
void Sonic::SonicUI::loadNextTrack(bool shuffle) {
  std::lock_guard<std::mutex> lock(mtx);
  // i don't even know if this good or not
  int nextAudioTrackIndex;
  // load next track
  if (!shuffle)
    nextAudioTrackIndex = m_CurrentTrack.queueindex + 1;
  else {

    // bad shuffle
    nextAudioTrackIndex = m_CurrentTrack.queueindex;
    srand(m_CurrentTrack.audio.duration);
    while (nextAudioTrackIndex == m_CurrentTrack.queueindex)
      nextAudioTrackIndex = rand() % AudioQueue.size();
  }
  if (nextAudioTrackIndex >= (int)AudioQueue.size()) {
    nextAudioTrackIndex = 0;
  }
  m_NextTrack.queueindex = nextAudioTrackIndex;
  m_NextTrack.audio = AudioQueue[m_NextTrack.queueindex];
}
std::string Sonic::SonicUI::getSelectionHeader() {

  switch (m_TrackSelector) {

  case TrackSelection::ARTIST: {
    return "Artists";
    break;
  }
  case TrackSelection::ALL_TRACKS: {
    return "All Tracks";
  }
  case TrackSelection::ALBUM: {
    return "Albums";
  }
  case TrackSelection::PLAYLIST: {
    return "Playlists";
  }
  }
}
void Sonic::SonicUI::playNextTrack(void) {
  std::unique_lock<std::mutex> lock(mtx);
  if (!AudioPlayer.m_Loop) {
    m_PrevTrack = m_CurrentTrack;
    m_CurrentTrack = m_NextTrack;
  }

  AudioPlayer.TogglePlay(m_CurrentTrack.audio);
  lock.unlock();
  loadNextTrack(AudioPlayer.m_Shuffle);
}
