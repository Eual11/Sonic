#ifndef _SONICUI_HPP
#define _SONICUI_HPP
#include "../include/AudioPlayer.hpp"
#include "../include/Utils.hpp"
#ifdef _WIN32
#include <Windows.h>
#endif

#include <cstdlib>
#include <filesystem>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/node.hpp>
#include <functional>
#include <mutex>
#include <set>
#include <string>
#include <thread>

struct TrackInfo {
  int albumIndex = 0;
  int queueindex = 0;
  Sonic::SonicAudio audio;
};
enum class TrackSelection {

  ARTIST,
  ALBUM,
  ALL_TRACKS,
  PLAYLIST
};

namespace Sonic {
struct SonicUIOptions {

  int freq;
  int channels;
  SDL_AudioFormat format;
  std::string new_path = "";
};
class SonicUI : public ftxui::ComponentBase {

private:
  int m_LeftPanelSelected = 0;
  bool m_StartedPlaying = false;
  std::atomic<bool> refresh_audio_queue = true;
  std::mutex mtx;
  int m_MainWindowSelected = 0;
  TrackInfo m_CurrentTrack;
  TrackInfo m_NextTrack;
  TrackInfo m_PrevTrack;

  TrackSelection m_TrackSelector;
  ftxui::Component LeftPanel =
      ftxui::Container::Vertical({}, &m_LeftPanelSelected);
  ftxui::Component MainWindow =
      ftxui::Container::Vertical({}, &m_MainWindowSelected);

  ftxui::Component StatusLine = ftxui::Container::Horizontal({});
  std::vector<std::string> LeftPanelSelection;
  std::vector<Sonic::SonicAudio> TracksList;
  std::vector<Sonic::SonicAudio> AudioQueue;
  void Load_Libraries(std::filesystem::path);
  void AudioQueueHandler(void);
  std::function<void()> quit;

  std::thread AudioHandlerThread;
  void UpdataLeftPanelSelection(void);
  void UpdateAudioQueue(int index);
  void UpdateLeftPanelView(void);
  void UpdateAllSelection(void);
  void UpdateMainWindowView(void);
  void loadNextTrack(bool);
  void playNextTrack(void);
#ifdef _WIN32
  void registerHotKeys(void);
  void handleHotKeys(ftxui::Event &);
#endif
  std::string getSelectionHeader();
  ftxui::MenuEntryOption LeftPanelEntryOption;
  ftxui::MenuEntryOption MainWindowEntryOption;
  SonicAudioPlayer AudioPlayer;
  ftxui::Color hexToRGB(std::string hex) {

    int i = 0;
    uint32_t rgb = 0x00;
    for (std::string::reverse_iterator iter = hex.rbegin(); iter != hex.rend();
         iter++, i++) {
      unsigned int val = 0;
      if (std::isalpha(*iter) && std::tolower(*iter) >= 'a' &&
          std::tolower(*iter) <= 'f') {

        val = 10 + std::tolower(*iter) - 'a';

      } else if (std::tolower(*iter) >= '0' && std::tolower(*iter) <= '9') {
        val = *iter - '0';
      } else
        val = 0;
      rgb += val * std::pow(16, i);
    }
    return ftxui::Color(rgb >> 16, (rgb & 0xff00) >> 8, rgb & 0xff);
    /* return rgb >> 8 & 0x00ff00; */
  }

public:
  SonicUI(SonicUIOptions); // constructor might need options
  bool OnEvent(ftxui::Event) override;
  ftxui::Element Render() override;
  inline void setQuitFunction(std::function<void()> q) { quit = std::move(q); }
};

} // namespace Sonic

#endif
