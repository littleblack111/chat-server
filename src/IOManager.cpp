#include "IOManager.hpp"
#include "inputManager.hpp"
#include "log.hpp"

CIOManager::CIOManager() {
	log(LOG, "IOManager: initialized");
}

CIOManager::~CIOManager() {
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_vIO.clear();
	}
	log(SYS, "IOManager: bye");
}

std::vector<CIOManager::SIO> CIOManager::getIO() const {
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_vIO;
}

void CIOManager::addLog(const SLog &log) {
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_vIO.push_back({.msg = std::nullopt, .log = log});
	}
	if (g_pInputManager)
		g_pInputManager->updateIO();
}
