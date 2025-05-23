#include "renderManager.hpp"
#include "log.hpp"

#ifndef NO_UI
CRenderManager::CRenderManager()
	: screen(ftxui::ScreenInteractive::TerminalOutput()) {
	if (std::atexit([]() {
			if (g_pRenderManager)
				g_pRenderManager->exitLoop();
		}))
		log(ERR, "Failed to register RenderManager atexit handler, will fallback to quick_exit after hang");
	log(LOG, "RenderManager: initialized");
}

CRenderManager::~CRenderManager() {
	exitLoop();
	log(SYS, "RenderManager: bye");
}

void CRenderManager::exitLoop() {
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
	screen.Loop(renderer);
}

void CRenderManager::setRenderer(ftxui::Component component) {
	renderer = std::move(component);
}

#else

CRenderManager::CRenderManager() {
	log(LOG, "Console RenderManager: initialized");
}

CRenderManager::~CRenderManager() {
	log(SYS, "Console RenderManager: bye");
}

void CRenderManager::enterLoop() {
	running = true;
}

void CRenderManager::exitLoop() {
	running = false;
}

#endif
