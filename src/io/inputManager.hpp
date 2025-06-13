#pragma once

#include "../misc/memory.hpp"
#include <ftxui/component/component.hpp>

class CInputManager {
  public:
	CInputManager();
	~CInputManager();

	void inputLoop();

	void updateIO();

  private:
	std::string m_szInput;

	ftxui::Component m_input;
	ftxui::Component inputComponent;
	ftxui::Component logComponent;
	ftxui::Component container;

	int bottomSize = 1;
};
inline UP<CInputManager> g_pInputManager;
