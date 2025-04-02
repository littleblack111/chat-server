#include "IOManager.hpp"
#include "inputManager.hpp"
#include "log.hpp"

CIOManager::CIOManager() {
	log(LOG, "IOManager: initialized");
}

CIOManager::~CIOManager() {
	m_vIO.clear();
	log(SYS, "IOManager: bye");
}

const std::vector<CIOManager::SIO> &CIOManager::getIO() const {
	return m_vIO;
}

void CIOManager::addLog(const SLog &log) {
	m_vIO.push_back({.msg = std::nullopt, .log = log});
	if (g_pInputManager)
		g_pInputManager->updateIO();
}
