#pragma once

#include <ftxui/component/component.hpp>
#include <memory>

class CInputManager {
  public:
	CInputManager();
	~CInputManager();

	void inputLoop();

  private:
	std::string m_szInput;

	ftxui::Component m_input;
	ftxui::Component inputComponent;
	ftxui::Component logComponent;
	ftxui::Component container;

	int bottomSize = 1;
};
inline std::unique_ptr<CInputManager> g_pInputManager;
