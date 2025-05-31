#pragma once

#include "FileDescriptor.hpp"
#include <memory>
#include <netinet/in.h>

class CServer {
  public:
	CServer(uint16_t port);
	~CServer();
	const CFileDescriptor *getSocket() const;

  private:
	CFileDescriptor m_sockfd;
	sockaddr_in		m_addr;

	mutable std::mutex m_mutex;
};
inline std::unique_ptr<CServer> g_pServer;
