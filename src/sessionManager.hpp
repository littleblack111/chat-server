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
  
	std::pair<std::jthread, std::shared_ptr<CSession>> *newSession(Hyprutils::OS::CFileDescriptor sockfd);

  void                       run();
	void												broadcast(const std::string &msg, eFormatType type = NONE) const;
	void												kick(const CSession *session) const;
	void												addSession(const CSession &session);
	void												removeSession(std::pair<std::jthread, std::shared_ptr<CSession>> *session);

  bool           nameExists(const std::string &name);

  private:
	void												broadcastChat(const std::string &msg, const std::string &username) const;

	std::vector<std::pair<std::jthread, std::shared_ptr<CSession>>> m_vSessions;
  friend class CChatManager;
};
inline std::shared_ptr<CSessionManager> g_pSessionManager;
