#include "sessionManager.hpp"
#include "log.hpp"
#include "session.hpp"
#include <algorithm>
#include <cstring>
#include <unistd.h>
#include <utility>

void CSessionManager::shutdownSessions() {
	for (auto &session : m_vSessions)
		kick(&session, false, "Server shutting down");
}

CSessionManager::CSessionManager() {
	if (std::atexit([]() { g_pSessionManager->shutdownSessions(); }))
		log(ERR, "Failed to register SessionManager atexit handler, will fallback to quick_exit after hang");

	log(LOG, "SessionManager: initialized");
};

CSessionManager::~CSessionManager() {
	for (auto &[thread, session] : m_vSessions) {
		if (thread.joinable())
			thread.detach();
		session.reset();
	}
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

void CSessionManager::broadcast(const std::string &msg) const {
	for (const auto &[thread, session] : m_vSessions)
		session->write(msg);
}

void CSessionManager::broadcastChat(const std::string &msg, const std::string &username) const {
	for (const auto &[thread, session] : m_vSessions) {
		if (!session)
			continue;
		auto name = session->getName();
		if (name != username)
			session->write(msg);
	}
}

bool CSessionManager::nameExists(const std::string &name) {
	return std::ranges::any_of(m_vSessions, [&name](const auto &s) { return s.second->getName() == name; });
}

CSession *CSessionManager::getByName(const std::string &name) const {
	auto it = std::ranges::find_if(m_vSessions, [&name](const auto &s) { return s.second->getName() == name; });
	return it != m_vSessions.end() ? it->second.get() : nullptr;
}

CSession *CSessionManager::getByIp(const char m_ip[INET_ADDRSTRLEN]) const {
	auto it = std::ranges::find_if(m_vSessions, [&m_ip](const auto &s) { log(LOG, s.second->m_ip); return strcmp(s.second->m_ip, m_ip) == 0; });
	return it != m_vSessions.end() ? it->second.get() : nullptr;
}

void CSessionManager::kick(CSession *session, const bool &kill, const std::string &reason) {
	for (auto &s : m_vSessions)
		if (s.second.get() == session)
			kick(&s, kill, reason);
}

void CSessionManager::kick(std::pair<std::jthread, std::shared_ptr<CSession>> *session, const bool &kill, const std::string &reason) {
  if (!reason.empty())
    session->second->write(reason);

	auto it = std::ranges::find_if(m_vSessions, [session](const auto &s) { return s.second.get() == session->second.get(); });

	if (it != m_vSessions.end()) {
		const auto native_handle = it->first.native_handle();
		if (it->first.joinable())
			it->first.detach(); // .detach the thread since it's removing itself
		it->second.reset();
		m_vSessions.erase(it);
		if (kill && native_handle != 0)
			pthread_cancel(native_handle);
	}
}
