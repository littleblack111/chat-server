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
    
#ifndef NO_UI
    if (g_pInputManager)
        g_pInputManager->updateIO();
#else
    // In console mode, we directly print the log
    std::println(log.type == ERR ? stderr : stdout, "{}", log.log);
#endif
}
