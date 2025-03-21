#pragma once

#include "format.hpp"
#include "session.hpp"
#include <memory>
#include <thread>
#include <vector>
#include "chatManager.hpp"

class CSessionManager {
  public:
	CSessionManager();
	~CSessionManager();

	std::pair<std::jthread, std::shared_ptr<CSession>> *newSession();

	void run();
	void broadcast(const std::string &msg) const;
	void kick(CSession *session, const bool &kill = false, const std::string &reason = "");
	void kick(std::pair<std::jthread, std::shared_ptr<CSession>> *session, const bool &kill = false, const std::string &reason = "");
	void addSession(const CSession &session);

	bool nameExists(const std::string &name);
	void shutdownSessions();

	CSession							  *getByName(const std::string &name) const;
	CSession							  *getByIp(const char m_ip[INET_ADDRSTRLEN]) const;
	std::vector<std::shared_ptr<CSession>> getSessions() const;

  private:
	void broadcastChat(const CChatManager::SMessage &msg) const;

	std::vector<std::pair<std::jthread, std::shared_ptr<CSession>>> m_vSessions;

	friend class CChatManager;
};
inline std::shared_ptr<CSessionManager> g_pSessionManager;
