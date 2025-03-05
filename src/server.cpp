#include "server.hpp"
#include "sessionManager.hpp"
#include <cstring>
#include <print>
#include <sys/socket.h>

CServer::CServer(uint16_t port)
	: m_sockfd(Hyprutils::OS::CFileDescriptor{socket(AF_INET, SOCK_STREAM, 0)}) {
	if (!m_sockfd.isValid())
		throw std::runtime_error("Failed to create socket");

	int reuse = 1;
	if (setsockopt(m_sockfd.get(), SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
		std::println(stderr, "setsockopt(SO_REUSEADDR) failed");

	memset(&m_addr, 0, sizeof(m_addr));
	m_addr.sin_family	   = AF_INET;
	m_addr.sin_port		   = htons(port);
	m_addr.sin_addr.s_addr = INADDR_ANY;
	bind(m_sockfd.get(), reinterpret_cast<sockaddr *>(&m_addr), sizeof(m_addr)) ||
			listen(m_sockfd.get(), 5)
		? throw std::runtime_error("Failed to bind or listen")
		: std::println("Server started on port: {}", port);
};

CServer::~CServer() {
	m_sockfd.reset();
}

void CServer::log(const eLogLevel loglevel, const std::string &message) const {
	std::println("[{}]: {}", "a", message);
}

Hyprutils::OS::CFileDescriptor CServer::getSocket() const {
	return m_sockfd.duplicate();
}
