#include "sessionManager.hpp"
#include "IOManager.hpp"
#include "chatServer.hpp"
#include "format.hpp"
#include "log.hpp"
#include "session.hpp"
#include <algorithm>
#include <mutex>
#include <ranges>
#include <unistd.h>
#include <utility>

void CSessionManager::shutdownSessions() {
	std::unique_lock<std::mutex> lock(m_mutex);
	if (m_vSessions.empty())
		return;

	for (auto &session : m_vSessions) {
		lock.unlock();
		kick(&session, false, "Server shutting down");
		lock.lock();
	}
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
	std::lock_guard<std::mutex> lock(m_mutex);

	for (auto &[thread, session] : m_vSessions) {
		if (thread.joinable())
			thread.detach();
		kick(session.get());
	}

	log(SYS, "SessionManager: bye");
}

std::pair<std::jthread, std::shared_ptr<CSession>> *CSessionManager::newSession() {
	std::shared_ptr<CSession>	 session = std::make_shared<CSession>();
	std::unique_lock<std::mutex> lock(m_mutex);
	const auto					 instance = &m_vSessions.emplace_back(std::jthread(&CSession::run, session.get()), session);
	lock.unlock();
	instance->second->setSelf(instance);
	return instance;
}

void CSessionManager::run() {
	while (true)
		newSession();
}

void CSessionManager::broadcast(const std::string &msg, std::optional<const CSession *const> self) const {
	std::unique_lock<std::mutex> lock(m_mutex);
	for (const auto &[thread, session] : m_vSessions) {
		lock.unlock();
		if (!self || (self && session.get() != *self))
			session->write(msg);
		lock.lock();
	}

	g_pIOManager->addCustom({"", msg});
}

void CSessionManager::broadcast(const CChatManager::SMessage &msg) const {
	std::unique_lock<std::mutex> lock(m_mutex);
	for (const auto &[thread, session] : m_vSessions) {
		lock.unlock();
		if (!session)
			continue;

		session->writeChat(msg);
		lock.lock();
	}

	g_pIOManager->addMessage(msg);
}

bool CSessionManager::nameExists(const std::string &name) {
	std::unique_lock<std::mutex> lock(m_mutex);
	auto						 it = std::ranges::find_if(m_vSessions, [&name](const auto &s) { return s.second->getName() == name; });
	lock.unlock();

	if (it != m_vSessions.end()) {
		if (!it->second->isValid()) {
			kick(it->second.get(), true, "Connection lost");
			return false;
		}
		return true;
	}
	return false;
}

CSession *CSessionManager::getByName(const std::string &name) const {
	std::lock_guard<std::mutex> lock(m_mutex);

	const auto it = std::ranges::find_if(m_vSessions, [&name](const auto &s) { return s.second->getName() == name; });
	return it != m_vSessions.end() ? it->second.get() : nullptr;
}

CSession *CSessionManager::getByIp(const std::string &ip) const {
	std::lock_guard<std::mutex> lock(m_mutex);

	const auto it = std::ranges::find_if(m_vSessions, [&ip](const auto &s) { log(LOG, s.second->m_ip); return s.second->m_ip == ip; });
	return it != m_vSessions.end() ? it->second.get() : nullptr;
}

CSession *CSessionManager::getByPtr(const uintptr_t &p) const {
	std::lock_guard<std::mutex> lock(m_mutex);

	const auto it = std::ranges::find_if(m_vSessions, [&p](const auto &s) { return (uintptr_t)s.second.get() == p; });
	return it != m_vSessions.end() ? it->second.get() : nullptr;
}

std::vector<std::shared_ptr<CSession>> CSessionManager::getSessions() const {
	std::lock_guard<std::mutex> lock(m_mutex);

	return m_vSessions | std::views::transform([](const auto &s) { return s.second; }) | std::ranges::to<std::vector>();
}

void CSessionManager::kick(CSession *session, const bool kill, const std::string &reason) {
	std::unique_lock<std::mutex> lock(m_mutex);

	for (auto &_session : m_vSessions) {
		lock.unlock();
		if (_session.second.get() == session)
			kick(&_session, kill, reason);
		lock.lock();
	}
}

void CSessionManager::kick(std::pair<std::jthread, std::shared_ptr<CSession>> *session, const bool kill, const std::string &reason) {
	std::unique_lock<std::mutex> lock(m_mutex);
	auto						 it = std::ranges::find_if(m_vSessions, [session](const auto &s) { return s.second.get() == session->second.get(); });
	lock.unlock();

	if (session->second)
		// this only exist when the session is registered
		// not sure why the second is null, but m_name definately is since it's setted during register
		// but we don't need to notify people if the session didn't even "join"/register anyways
		g_pSessionManager->broadcast(NFormatter::fmt(NONEWLINE, "{} has left the chat", session->second->m_name), session->second.get());

	lock.lock();
	if (it != m_vSessions.end()) {
		lock.unlock();
		if (!reason.empty()) {
			lock.unlock();
			it->second->setDeaf(false);
			it->second->write(reason);
		}
		const auto native_handle = it->first.native_handle();
		if (it->first.joinable())
			it->first.detach(); // .detach the thread since it's removing itself
		it->second.reset();
		lock.lock();
		m_vSessions.erase(it);
		lock.unlock();
		if (kill && native_handle != 0)
			// cancel(terminal/kill) the thread if it doesn't exit on its own
			// pthread_cancel cuz it's the safest https://stackoverflow.com/a/3438576
			// need a condition because it might crash if the thread is already dead
			pthread_cancel(native_handle);
	}
}
