#include "../include/SonicUI.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/node.hpp>

Sonic::SonicUI::SonicUI() {

  ftxui::Component left_main =
      ftxui::Container::Horizontal({LeftPanel, MainWindow});
  ftxui::Component sonicui =
      ftxui::Container::Vertical({left_main, StatusLine});

  Add(sonicui);
}

bool Sonic::SonicUI::OnEvent(ftxui::Event event) {

  return ftxui::ComponentBase::OnEvent(event);
}

ftxui::Element Sonic::SonicUI::Render() {

  ftxui::Element mainView = ftxui::hbox(
      {LeftPanel->Render() | ftxui::borderLight | ftxui::yframe |
           ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 30),
       MainWindow->Render() | ftxui::yframe | ftxui::vscroll_indicator |
           ftxui::flex | ftxui::borderLight});

  ftxui::Element statusline_view =
      ftxui::hbox(
          {ftxui::text("this is empty") | ftxui::flex | ftxui::borderLight}) |
      ftxui::size(ftxui::HEIGHT, ftxui::EQUAL, 5);

  return ftxui::vbox({mainView | ftxui::flex, statusline_view});
}
