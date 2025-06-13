#include "sessionManager.hpp"
#include "../debug/format.hpp"
#include "../debug/log.hpp"
#include "../io/IOManager.hpp"
#include "chatServer.hpp"
#include "session.hpp"
#include <algorithm>
#include <ranges>
#include <unistd.h>
#include <utility>

void CSessionManager::shutdownSessions() {
	if (m_vSessions.empty())
		return;

	for (auto &[thread, session] : m_vSessions) {
		if (session) {
			session->setDeaf(false);
			session->write("Server shutting down");
			if (!session->getName().empty())
				g_pSessionManager->broadcast(NFormatter::fmt(NONEWLINE, "{} has left the chat", session->getName()), std::weak_ptr<CSession>(session));
		}
	}

	for (auto &[thread, session] : m_vSessions) {
		if (thread.joinable())
			thread.detach();
		session.reset();
	}

	m_vSessions.clear();
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
		kick(std::weak_ptr<CSession>(session));
	}

	log(SYS, "SessionManager: bye");
}

void CSessionManager::newSession() {
	std::shared_ptr session	 = std::make_shared<CSession>();
	auto		   &instance = m_vSessions.emplace_back(std::jthread([session]() { session->run(); }), session);
	instance.second->self	 = std::weak_ptr<CSession>(instance.second);
}

void CSessionManager::run() {
	while (true)
		newSession();
}

void CSessionManager::broadcast(const std::string &msg, std::optional<std::weak_ptr<CSession>> self) const {
	for (const auto &[thread, session] : m_vSessions)
		if (!self || (self && session != self->lock()))
			session->write(msg);

	g_pIOManager->addCustom({"", msg});
}

void CSessionManager::broadcast(const CChatManager::SMessage &msg) const {
	for (const auto &[thread, session] : m_vSessions) {
		if (!session)
			continue;

		session->writeChat(msg);
	}

	g_pIOManager->addMessage(msg);
}

bool CSessionManager::nameExists(const std::string &name) {
	auto it = std::ranges::find_if(m_vSessions, [&name](const auto &s) { return s.second->getName() == name; });

	if (it != m_vSessions.end()) {
		if (!it->second->isValid()) {
			kick(std::weak_ptr<CSession>(it->second), true, "Connection lost");
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
	auto it = std::ranges::find_if(m_vSessions, [&ip](const auto &s) { log(LOG, s.second->getIp()); return s.second->getIp() == ip; });
	return it != m_vSessions.end() ? it->second : nullptr;
}

std::vector<std::shared_ptr<CSession>> CSessionManager::getSessions() const {
	return m_vSessions | std::views::transform([](const auto &s) { return s.second; }) | std::ranges::to<std::vector>();
}

void CSessionManager::kick(std::weak_ptr<CSession> session_weak, const bool kill, const std::string &reason) {
	auto session = session_weak.lock();
	if (!session)
		return;

	for (auto it = m_vSessions.begin(); it != m_vSessions.end(); ++it) {
		if (it->second == session) {
			if (session && !session->getName().empty())
				// this only exist when the session is registered
				// not sure why the second is null, but m_name definately is since it's setted during register
				// but we don't need to notify people if the session didn't even "join"/register anyways
				g_pSessionManager->broadcast(NFormatter::fmt(NONEWLINE, "{} has left the chat", session->getName()), session_weak);

			if (!reason.empty()) {
				session->setDeaf(false);
				session->write(reason);
			}
			const auto native_handle = it->first.native_handle();
			if (it->first.joinable())
				it->first.detach(); // .detach the thread since it's removing itself
			it->second.reset();
			m_vSessions.erase(it);
			if (kill && native_handle != 0)
				// cancel(terminal/kill) the thread if it doesn't exit on its own
				// pthread_cancel cuz it's the safest https://stackoverflow.com/a/3438576
				// need a condition because it might crash if the thread is already dead
				pthread_cancel(native_handle);
			return;
		}
	}
}
