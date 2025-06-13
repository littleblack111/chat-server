#include "IOManager.hpp"
#include "../debug/log.hpp"
#include "inputManager.hpp"

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
	m_vIO.push_back({.log = log});
	if (g_pInputManager)
		g_pInputManager->updateIO();
}

void CIOManager::addMessage(const CChatManager::SMessage &msg) {
	m_vIO.push_back({.msg = msg});
	if (g_pInputManager)
		g_pInputManager->updateIO();
}

void CIOManager::addCustom(const std::pair<std::string, std::string> &custom) {
	m_vIO.push_back({.custom = custom});
	if (g_pInputManager)
		g_pInputManager->updateIO();
}
