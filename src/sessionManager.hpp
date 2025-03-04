#pragma once

#include "session.hpp"
#include <memory>
#include <thread>
#include <vector>

class CSessionManager {
  public:
	CSessionManager();
	~CSessionManager();
	void enterLoop();
	void newSession(Hyprutils::OS::CFileDescriptor sockfd);
	void broadcast(const std::string &message) const;
	void kick(const CSession *session) const;
	void addSession(const CSession &session);

  private:
	std::vector<std::pair<std::jthread, std::shared_ptr<CSession>>> m_vSessions;
};
inline std::unique_ptr<CSessionManager> g_pSessionManager;
