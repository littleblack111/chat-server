#include "renderManager.hpp"

CRenderManager::CRenderManager()
	: screen(ftxui::ScreenInteractive::Fullscreen()) {
}

void CRenderManager::enterLoop() {
	screen.Loop(renderer);
}

void CRenderManager::setRenderer(ftxui::Component component) {
	renderer = std::move(component);
}

CRenderManager::~CRenderManager() {
	screen.Exit();
}
