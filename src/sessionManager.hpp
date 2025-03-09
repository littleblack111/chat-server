#pragma once

#include "format.hpp"
#include "session.hpp"
#include <memory>
#include <thread>
#include <vector>

class CSessionManager {
  public:
	CSessionManager();
	~CSessionManager();
	void												enterLoop();
	std::pair<std::jthread, std::shared_ptr<CSession>> *newSession(Hyprutils::OS::CFileDescriptor sockfd);
	void												broadcast(const std::string &message, eFormatType type = NONE) const;
	void												kick(const CSession *session) const;
	void												addSession(const CSession &session);
	void												removeSession(std::pair<std::jthread, std::shared_ptr<CSession>> *session);

  private:
	std::vector<std::pair<std::jthread, std::shared_ptr<CSession>>> m_vSessions;
};
inline std::shared_ptr<CSessionManager> g_pSessionManager;
