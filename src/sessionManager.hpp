#pragma once

#include "chatManager.hpp"
#include "format.hpp"
#include "session.hpp"
#include <memory>
#include <thread>
#include <vector>

class CSessionManager {
  public:
	CSessionManager();
	~CSessionManager();

	void newSession();

	void run();
	void broadcast(const std::string &msg) const;
	void kick(std::shared_ptr<CSession> session, const bool kill = false, const std::string &reason = "");
	void kick(std::unique_ptr<std::pair<std::jthread, std::shared_ptr<CSession>>> session, const bool kill = false, const std::string &reason = "");
	void addSession(const CSession &session);

	bool nameExists(const std::string &name);
	void shutdownSessions();

	std::shared_ptr<CSession>			   getByName(const std::string &name) const;
	std::shared_ptr<CSession>			   getByIp(const std::string &ip) const;
	std::vector<std::shared_ptr<CSession>> getSessions() const;

  private:
	void broadcastChat(const CChatManager::SMessage &msg) const;

	std::vector<std::pair<std::jthread, std::shared_ptr<CSession>>> m_vSessions;

	friend class CChatManager;
};
inline std::shared_ptr<CSessionManager> g_pSessionManager;
