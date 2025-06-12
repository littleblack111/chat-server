#pragma once

#include "../debug/format.hpp"
#include "../misc/memory.hpp"
#include "chatManager.hpp"
#include "session.hpp"
#include <thread>
#include <vector>

class CSessionManager {
  public:
	CSessionManager();
	~CSessionManager();

	void newSession();

	void run();
	void broadcast(const std::string &msg, std::optional<WP<CSession>> self = std::nullopt) const; // second param only specified when we want to exclude the sender
	void kick(WP<CSession> session, const bool kill = false, const std::string &reason = "");
	void addSession(const CSession &session);

	bool nameExists(const std::string &name);
	void shutdownSessions();

	SP<CSession>			  getByName(const std::string &name) const;
	SP<CSession>			  getByIp(const std::string &ip) const;
	std::vector<SP<CSession>> getSessions() const;

  private:
	void broadcast(const CChatManager::SMessage &msg) const;

	std::vector<std::pair<std::jthread, SP<CSession>>> m_vSessions;

	friend class CChatManager;
};
inline SP<CSessionManager> g_pSessionManager;
