#include "sessionManager.hpp"
#include "log.hpp"
#include "session.hpp"
#include <algorithm>
#include <cstring>
#include <ranges>
#include <unistd.h>
#include <utility>

void CSessionManager::shutdownSessions() {
	if (m_vSessions.empty())
		return;

	for (auto &session : m_vSessions)
		kick(std::unique_ptr<std::pair<std::jthread, std::shared_ptr<CSession>>>(&session), false, "Server shutting down");
}

CSessionManager::CSessionManager() {
	if (std::atexit([]() {
			if (g_pSessionManager)
				g_pSessionManager->shutdownSessions();
		}))
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

void CSessionManager::newSession() {
	std::shared_ptr session	 = std::make_shared<CSession>();
	auto			instance = std::unique_ptr<std::pair<std::jthread, std::shared_ptr<CSession>>>(&m_vSessions.emplace_back(std::jthread(&CSession::run, session.get()), session));
	instance->second->setSelf(std::move(instance));
}

void CSessionManager::run() {
	while (true)
		newSession();
}

void CSessionManager::broadcast(const std::string &msg) const {
	for (const auto &[thread, session] : m_vSessions)
		session->write(msg);
}

void CSessionManager::broadcastChat(const CChatManager::SMessage &msg) const {
	for (const auto &[thread, session] : m_vSessions) {
		if (!session)
			continue;

		session->writeChat(msg);
	}
}

bool CSessionManager::nameExists(const std::string &name) {
	auto it = std::ranges::find_if(m_vSessions, [&name](const auto &s) { return s.second->getName() == name; });

	if (it != m_vSessions.end()) {
		if (!it->second->isValid()) {
			kick(it->second, true, "Connection lost");
			return false;
		}
		return true;
	}
	return false;
}

std::shared_ptr<CSession> CSessionManager::getByName(const std::string &name) const {
	auto it = std::ranges::find_if(m_vSessions, [&name](const auto &s) { return s.second->getName() == name; });
	return it != m_vSessions.end() ? it->second : nullptr;
}

std::shared_ptr<CSession> CSessionManager::getByIp(const std::string &ip) const {
	auto it = std::ranges::find_if(m_vSessions, [&ip](const auto &s) { log(LOG, s.second->m_ip); return s.second->m_ip == ip; });
	return it != m_vSessions.end() ? it->second : nullptr;
}

std::vector<std::shared_ptr<CSession>> CSessionManager::getSessions() const {
	return m_vSessions | std::views::transform([](const auto &s) { return s.second; }) | std::ranges::to<std::vector>();
}

void CSessionManager::kick(std::shared_ptr<CSession> session, const bool kill, const std::string &reason) {
	for (auto &_session : m_vSessions)
		if (_session.second == session)
			kick(std::unique_ptr<std::pair<std::jthread, std::shared_ptr<CSession>>>(&_session), kill, reason);
}

void CSessionManager::kick(std::unique_ptr<std::pair<std::jthread, std::shared_ptr<CSession>>> session, const bool kill, const std::string &reason) {
	auto it = std::ranges::find_if(m_vSessions, [&session](const auto &s) { return s.second.get() == session->second.get(); });

	if (it != m_vSessions.end()) {
		if (!reason.empty()) {
			it->second->setDeaf(false);
			it->second->write(reason);
		}
		const auto native_handle = it->first.native_handle();
		if (it->first.joinable())
			it->first.detach(); // .detach the thread since it's removing itself
		it->second.reset();
		m_vSessions.erase(it);
		if (kill && native_handle != 0)
			pthread_cancel(native_handle);
	}
}
