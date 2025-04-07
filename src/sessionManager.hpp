#pragma once

#include "chatManager.hpp"
#include "format.hpp"
#include "session.hpp"
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

class CSessionManager {
  public:
	CSessionManager();
	~CSessionManager();

	std::pair<std::jthread, std::shared_ptr<CSession>> *newSession();

	void run();
	void broadcast(const std::string &msg) const;
	void kick(CSession *session, const bool kill = false, const std::string &reason = "");
	void kick(std::pair<std::jthread, std::shared_ptr<CSession>> *session, const bool kill = false, const std::string &reason = "");
	void addSession(const CSession &session);

	bool nameExists(const std::string &name);
	void shutdownSessions();

	CSession							  *getByName(const std::string &name) const;
	CSession							  *getByIp(const std::string &ip) const;
	std::vector<std::shared_ptr<CSession>> getSessions() const;

  private:
	void broadcastChat(const CChatManager::SMessage &msg) const;

	mutable std::mutex												m_mutex;
	std::vector<std::pair<std::jthread, std::shared_ptr<CSession>>> m_vSessions;

	friend class CChatManager;
};
inline std::shared_ptr<CSessionManager> g_pSessionManager;
