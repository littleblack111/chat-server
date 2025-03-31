#include "renderManager.hpp"
#include "log.hpp"

CRenderManager::CRenderManager()
	: screen(ftxui::ScreenInteractive::Fullscreen()) {
	log(SYS, "RenderManager: initialized");
};

void CRenderManager::enterLoop() {
	screen.Loop(renderer);
}

void CRenderManager::setRenderer(ftxui::Component component) {
	renderer = std::move(component);
}

CRenderManager::~CRenderManager() {
	screen.Exit();
}
