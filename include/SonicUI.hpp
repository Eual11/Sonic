#ifndef _SONICUI_HPP
#define _SONICUI_HPP
#include "../include/AudioPlayer.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/node.hpp>
#include <set>
#include <string>

namespace Sonic {

class SonicUI : public ftxui::ComponentBase {

private:
  int m_LeftPanelSelected;
  int m_MainWindowSelected;

  ftxui::Component LeftPanel =
      ftxui::Container::Vertical({}, &m_LeftPanelSelected);
  ftxui::Component MainWindow =
      ftxui::Container::Vertical({}, &m_MainWindowSelected);

  ftxui::Component StatusLine = ftxui::Container::Horizontal({});
  std::set<std::string> LeftPanelSelection;
  std::vector<Sonic::SonicAudio> AudioSelection;

public:
  SonicUI(); // constructor might need options
  bool OnEvent(ftxui::Event) override;
  ftxui::Element Render() override;
};
} // namespace Sonic

#endif
