#include "sessionManager.hpp"
#include "server.hpp"
#include "session.hpp"
#include <algorithm>
#include <unistd.h>
#include <utility>

CSessionManager::CSessionManager() = default;

CSessionManager::~CSessionManager() {
	for (auto &[thread, session] : m_vSessions)
		thread.join();
}

std::pair<std::jthread, std::shared_ptr<CSession>> *CSessionManager::newSession(Hyprutils::OS::CFileDescriptor sockfd) {
	std::shared_ptr session	 = std::make_unique<CSession>(std::move(sockfd));
	const auto		instance = &m_vSessions.emplace_back(std::jthread(&CSession::run, session), std::move(session));
	instance->second->setSelf(instance);
	return instance;
}

void CSessionManager::enterLoop() {
	while (true)
		newSession(Hyprutils::OS::CFileDescriptor{accept(g_pServer->getSocket()->get(), nullptr, nullptr) /* awaits new client/session */});
}

void CSessionManager::broadcast(const std::string &msg, eFormatType type) const {
	for (const auto &[thread, session] : m_vSessions)
		session->write(msg, type);
}

void CSessionManager::broadcastChat(const std::string &msg, const std::string &username) const {
	for (const auto &[thread, session] : m_vSessions)
    if (session->getName() != username)
      session->write(msg, NONE);
}

void CSessionManager::removeSession(std::pair<std::jthread, std::shared_ptr<CSession>> *session) {
	auto it = std::ranges::find_if(m_vSessions, [session](const auto &s) { return s.second.get() == session->second.get(); });

	if (it != m_vSessions.end()) {
		if (it->first.joinable())
			it->first.detach(); // .detach the thread since it's removing itself
		m_vSessions.erase(it);
	}
}
