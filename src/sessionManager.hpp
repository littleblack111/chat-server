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
	void broadcast(const std::string &msg, std::optional<std::weak_ptr<CSession>> self = std::nullopt) const; // second param only specified when we want to exclude the sender
	void kick(std::weak_ptr<CSession> session, const bool kill = false, const std::string &reason = "");
	void addSession(const CSession &session);

	bool nameExists(const std::string &name);
	void shutdownSessions();

	std::shared_ptr<CSession>			   getByName(const std::string &name) const;
	std::shared_ptr<CSession>			   getByIp(const std::string &ip) const;
	std::vector<std::shared_ptr<CSession>> getSessions() const;

	// Safer weak_ptr versions - recommended for most use cases to avoid accidental lifetime extension
	std::weak_ptr<CSession> getByNameWeak(const std::string &name) const;
	std::weak_ptr<CSession> getByIpWeak(const std::string &ip) const;

  private:
	void broadcast(const CChatManager::SMessage &msg) const;

	std::vector<std::pair<std::jthread, std::shared_ptr<CSession>>> m_vSessions;

	friend class CChatManager;
};
inline std::shared_ptr<CSessionManager> g_pSessionManager;
