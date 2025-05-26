#pragma once

#include "FileDescriptor.hpp"
#include <memory>
#include <mutex>
#include <netinet/in.h>

class CServer {
  public:
	CServer(uint16_t port);
	~CServer();
	std::shared_ptr<CFileDescriptor> getSocket() const;

  private:
	std::shared_ptr<CFileDescriptor> m_sockfd;
	sockaddr_in						 m_addr;

	mutable std::mutex m_mutex;
};
inline std::unique_ptr<CServer> g_pServer;
