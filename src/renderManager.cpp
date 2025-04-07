#include "renderManager.hpp"
#include "log.hpp"

CRenderManager::CRenderManager()
	: screen(ftxui::ScreenInteractive::TerminalOutput()) {
	if (std::atexit([]() {
			if (g_pRenderManager)
				g_pRenderManager->exitLoop();
		}))
		log(ERR, "Failed to register RenderManager atexit handler, will fallback to quick_exit after hang");
	log(LOG, "RenderManager: initialized");
};

CRenderManager::~CRenderManager() {
	exitLoop();
	log(SYS, "RenderManager: bye");
}

void CRenderManager::exitLoop() {
	std::lock_guard<std::mutex> lock(m_mutex);
	if (renderer) {
		try {
			screen.ExitLoopClosure()();
			screen.Exit();
		} catch (const std::exception &e) {
			log(WARN, "Exception in exitLoop: {}", e.what());
		} catch (...) {
			log(WARN, "Unknown exception in exitLoop");
		}
	}
}

void CRenderManager::enterLoop() {
	std::lock_guard<std::mutex> lock(m_mutex);
	screen.Loop(renderer);
}

void CRenderManager::setRenderer(ftxui::Component component) {
	std::lock_guard<std::mutex> lock(m_mutex);
	renderer = std::move(component);
}
