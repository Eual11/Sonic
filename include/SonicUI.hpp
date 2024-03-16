#ifndef _SONICUI_HPP
#define _SONICUI_HPP
#include "../include/AudioPlayer.hpp"
#include <filesystem>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/node.hpp>
#include <set>
#include <string>

enum class TrackSelection {

  ARTIST,
  ALBUM,
  PLAYLIST
};

namespace Sonic {
struct SonicUIOptions {

  int freq;
  int channels;
  SDL_AudioFormat format;
  std::string new_path;
};
class SonicUI : public ftxui::ComponentBase {

private:
  int m_LeftPanelSelected = 0;
  int m_MainWindowSelected = 0;

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
  void UpdataLeftPanelSelection(void);
  void UpdateAudioQueue(int index);
  void UpdateLeftPanelView(void);
  void UpdateAllSelection(void);
  void UpdateMainWindowView(void);
  ftxui::MenuEntryOption LeftPanelEntryOption;
  ftxui::MenuEntryOption MainWindowEntryOption;
  SonicAudioPlayer AudioPlayer;

public:
  SonicUI(SonicUIOptions); // constructor might need options
  bool OnEvent(ftxui::Event) override;
  ftxui::Element Render() override;
};

} // namespace Sonic

#endif
