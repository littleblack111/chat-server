#pragma once

#include "FileDescriptor.hpp"
#include "log.hpp"
#include <memory>
#include <netinet/in.h>
#include <string>

class CServer {
  public:
	CServer(uint16_t port);
	~CServer();
	void						   log(const eLogLevel loglevel, const std::string &message) const;
	Hyprutils::OS::CFileDescriptor getSocket() const;

  private:
	Hyprutils::OS::CFileDescriptor m_sockfd;
	sockaddr_in					   m_addr;
};
inline std::unique_ptr<CServer> g_pServer;
