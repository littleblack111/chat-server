#pragma once

#include <memory>
#include <string>

#ifndef NO_UI
#include <ftxui/component/component.hpp>
#endif

class CInputManager {
  public:
	CInputManager();
	~CInputManager();

	void inputLoop();
	void updateIO();

  private:
	std::string m_szInput;

#ifndef NO_UI
	ftxui::Component m_input;
	ftxui::Component inputComponent;
	ftxui::Component logComponent;
	ftxui::Component container;

	int bottomSize = 1;
#endif
};
inline std::unique_ptr<CInputManager> g_pInputManager;
