#pragma once

#include <ftxui/component/screen_interactive.hpp>
#include <memory>
#include <mutex>

class CRenderManager {
  public:
	CRenderManager();
	~CRenderManager();

	void enterLoop();

	void setRenderer(ftxui::Component component);

  private:
	void exitLoop();

	std::mutex				 m_mutex;
	ftxui::ScreenInteractive screen;
	ftxui::Component		 renderer;

	friend class CInputManager;
};
inline std::unique_ptr<CRenderManager> g_pRenderManager;
