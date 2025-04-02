#pragma once

#include <ftxui/component/screen_interactive.hpp>
#include <memory>

class CRenderManager {
  public:
	CRenderManager();
	~CRenderManager();

	void enterLoop();

	void setRenderer(ftxui::Component component);

  private:
	ftxui::ScreenInteractive screen;
	ftxui::Component		 renderer;

  friend class CInputManager;
};
inline std::unique_ptr<CRenderManager> g_pRenderManager;
