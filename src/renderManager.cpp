#include "renderManager.hpp"
#include "log.hpp"

CRenderManager::CRenderManager()
	: screen(ftxui::ScreenInteractive::TerminalOutput()) {
	if (std::atexit([]() { g_pRenderManager->exitLoop(); }))
		log(ERR, "Failed to register SessionManager atexit handler, will fallback to quick_exit after hang");
	log(LOG, "RenderManager: initialized");
};

CRenderManager::~CRenderManager() {
	exitLoop();
	log(SYS, "RenderManager: bye");
}

void CRenderManager::exitLoop() {
	screen.ExitLoopClosure()();
	screen.Exit();
}

void CRenderManager::enterLoop() {
	screen.Loop(renderer);
}

void CRenderManager::setRenderer(ftxui::Component component) {
	renderer = std::move(component);
}
