#pragma once

#include "FileDescriptor.hpp"
#include <memory>
#include <netinet/in.h>

class CServer {
  public:
	CServer(uint16_t port);
	~CServer();
	const Hyprutils::OS::CFileDescriptor *getSocket() const;

  private:
	Hyprutils::OS::CFileDescriptor m_sockfd;
	sockaddr_in					   m_addr;
};
inline std::unique_ptr<CServer> g_pServer;
