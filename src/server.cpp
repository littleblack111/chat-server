#include "server.hpp"
#include "log.hpp"
#include <cstring>
#include <sys/socket.h>

CServer::CServer(uint16_t port)
	: m_sockfd(Hyprutils::OS::CFileDescriptor{socket(AF_INET, SOCK_STREAM, 0)}) {
	if (!m_sockfd.isValid())
		throw std::runtime_error("Failed to create socket");

	constexpr int opt  = 1;
	constexpr int size = sizeof(opt);
	if (setsockopt(m_sockfd.get(), SOL_SOCKET, SO_REUSEADDR, &opt, size) < 0)
		log(stderr, WARN, "setsockopt(SO_REUSEADDR) failed");

	if (setsockopt(m_sockfd.get(), SOL_SOCKET, SO_KEEPALIVE, &opt, size) < 0)
		log(stderr, WARN, "setsockopt(SO_KEEPALIVE) failed");

	memset(&m_addr, 0, sizeof(m_addr));
	m_addr.sin_family	   = AF_INET;
	m_addr.sin_port		   = htons(port);
	m_addr.sin_addr.s_addr = INADDR_ANY;
	bind(m_sockfd.get(), reinterpret_cast<sockaddr *>(&m_addr), sizeof(m_addr)) ||
			listen(m_sockfd.get(), 5)
		? throw std::runtime_error("Failed to bind or listen")
		: log(LOG, "Server started on port: {}", port);

	log(LOG, "Server: initialized");
};

CServer::~CServer() {
	m_sockfd.reset();
	log(SYS, "Server: bye");
}

const Hyprutils::OS::CFileDescriptor *CServer::getSocket() const {
	return &m_sockfd;
}
