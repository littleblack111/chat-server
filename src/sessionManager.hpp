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

	std::pair<std::jthread, std::shared_ptr<CSession>> *newSession();

	void run();
	void broadcast(const std::string &msg, std::optional<const CSession* const> self = std::nullopt) const; // second param only specified when we want to exclude the sender
	void kick(CSession *session, const bool kill = false, const std::string &reason = "");
	void kick(std::pair<std::jthread, std::shared_ptr<CSession>> *session, const bool kill = false, const std::string &reason = "");
	void addSession(const CSession &session);

	bool nameExists(const std::string &name);
	void shutdownSessions();

	CSession							  *getByName(const std::string &name) const;
	CSession							  *getByIp(const std::string &ip) const;
	CSession							  *getByPtr(const uintptr_t &p) const;
	std::vector<std::shared_ptr<CSession>> getSessions() const;

  private:
	void broadcast(const CChatManager::SMessage &msg) const;

	std::vector<std::pair<std::jthread, std::shared_ptr<CSession>>> m_vSessions;

	friend class CChatManager;
};
inline std::shared_ptr<CSessionManager> g_pSessionManager;
