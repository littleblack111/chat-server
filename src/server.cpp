#include "server.hpp"
#include "sessionManager.hpp"
#include <cstring>
#include <print>
#include <sys/socket.h>

CServer::CServer(uint16_t port)
	: m_sockfd(Hyprutils::OS::CFileDescriptor{socket(AF_INET, SOCK_STREAM, 0)}) {
	if (!m_sockfd.isValid())
		throw std::runtime_error("Failed to create socket");

	memset(&m_addr, 0, sizeof(m_addr));
	m_addr.sin_family	   = AF_INET;
	m_addr.sin_port		   = htons(port);
	m_addr.sin_addr.s_addr = INADDR_ANY;
	m_dBind				   = bind(m_sockfd.get(), reinterpret_cast<sockaddr *>(&m_addr), sizeof(m_addr));
	m_dListen			   = listen(m_sockfd.get(), 5);
	 m_dBind < 0 || m_dListen < 0 ? throw std::runtime_error("Failed to bind or listen") : std::println("Server started on port: {}", port);

	// char buffer[1024] = {0};
	// int clientSocket = accept(m_sockfd.get(), nullptr, nullptr);
	// recv(clientSocket, buffer, sizeof(buffer), 0);
	// std::println("Message from client: {}", buffer);
};

// void CServer::run() {
// 	while (true) {
// 		char buffer[1024] = {0};
// 		int	 clientSocket = accept(m_sockfd.get(), nullptr, nullptr);
// 		recv(clientSocket, buffer, sizeof(buffer), 0);
// 		std::println("Message from client: {}", buffer);
// 	}
// }

CServer::~CServer() {
	m_sockfd.reset();
}

void CServer::log(const eLogLevel loglevel, const std::string &message) const {
	std::println("[{}]: {}", "a", message);
}

bool CServer::isValid() {
	return m_sockfd.isValid() && m_dBind >= 0 && m_dListen >= 0;
}

Hyprutils::OS::CFileDescriptor CServer::getSocket() const {
	return m_sockfd.duplicate();
}
