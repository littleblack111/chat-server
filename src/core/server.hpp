#pragma once

#include "../misc/FileDescriptor.hpp"
#include "../misc/memory.hpp"
#include <netinet/in.h>

class CServer {
  public:
	CServer(uint16_t port);
	~CServer();
	SP<CFileDescriptor> getSocket() const;

  private:
	SP<CFileDescriptor> m_sockfd;
	sockaddr_in			m_addr;

	mutable std::mutex m_mutex;
};
inline UP<CServer> g_pServer;
