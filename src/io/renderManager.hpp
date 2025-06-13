#pragma once

#include <ftxui/component/screen_interactive.hpp>
#include "../misc/memory.hpp"

class CRenderManager {
  public:
	CRenderManager();
	~CRenderManager();

	void enterLoop();

	void setRenderer(ftxui::Component component);

  private:
	void exitLoop();

	ftxui::ScreenInteractive screen;
	ftxui::Component		 renderer;

	friend class CInputManager;
};
inline UP<CRenderManager> g_pRenderManager;
