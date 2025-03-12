#include "sessionManager.hpp"
#include "log.hpp"
#include "session.hpp"
#include <algorithm>
#include <unistd.h>
#include <utility>

void CSessionManager::shutdownSessions() {
	for (const auto &[thread, session] : m_vSessions)
		session->onShutdown();
}

CSessionManager::CSessionManager() {
	if (std::atexit([]() { g_pSessionManager->shutdownSessions(); }))
		log(ERR, "Failed to register SessionManager atexit handler, will fallback to quick_exit after hang");

	log(LOG, "SessionManager: initialized");
};

CSessionManager::~CSessionManager() {
	for (auto &[thread, session] : m_vSessions)
		if (thread.joinable())
			thread.join();

	log(SYS, "SessionManager: bye");
}

std::pair<std::jthread, std::shared_ptr<CSession>> *CSessionManager::newSession() {
	std::shared_ptr session	 = std::make_shared<CSession>();
	const auto		instance = &m_vSessions.emplace_back(std::jthread(&CSession::run, session.get()), session);
	instance->second->setSelf(instance);
	return instance;
}

void CSessionManager::run() {
	while (true)
		newSession();
}

void CSessionManager::broadcast(const std::string &msg, eFormatType type) const {
	for (const auto &[thread, session] : m_vSessions)
		session->write(msg, type);
}

void CSessionManager::broadcastChat(const std::string &msg, const std::string &username) const {
	for (const auto &[thread, session] : m_vSessions) {
		if (!session)
			continue;
		auto name = session->getName();
		if (name != username)
			session->write(msg, NONE);
	}
}

bool CSessionManager::nameExists(const std::string &name) {
	return std::ranges::any_of(m_vSessions, [&name](const auto &s) { return s.second->getName() == name; });
}

void CSessionManager::removeSession(std::pair<std::jthread, std::shared_ptr<CSession>> *session) {
	auto it = std::ranges::find_if(m_vSessions, [session](const auto &s) { return s.second.get() == session->second.get(); });

	if (it != m_vSessions.end()) {
		if (it->first.joinable())
			it->first.detach(); // .detach the thread since it's removing itself
		m_vSessions.erase(it);
	}
}
