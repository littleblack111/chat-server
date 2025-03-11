#include "session.hpp"
#include "chatManager.hpp"
#include "log.hpp"
#include "server.hpp"
#include "sessionManager.hpp"
#include <cerrno>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>

CSession::CSession() : m_addrLen(sizeof(m_addr)) {
  m_sockfd = Hyprutils::OS::CFileDescriptor{accept(g_pServer->getSocket()->get(), reinterpret_cast<sockaddr *>(&m_addr), &m_addrLen)}; // if this is in the init list, it will run before m_addrLen, so it won't work :/

	if (!m_sockfd.isValid())
		throw std::runtime_error("session: Failed to create socket");

  inet_ntop(AF_INET, &m_addr.sin_addr, m_ip, INET_ADDRSTRLEN);
  m_port = ntohs(m_addr.sin_port);
  log(TRACE, "session({}): connected on port {}", m_ip, m_port);

	log(TRACE, "session: initialized");
}

CSession::~CSession() {
	onDisconnect();
	m_sockfd.reset();

	self->second.reset();
	log(TRACE, "session({}): bye", m_name);
}

void CSession::onConnect() {
	log(LOG, "Client connected");
}

void CSession::onDisconnect() {
	log(LOG, "Client disconnected");
}

void CSession::onErrno(eEventType eventType) {
	const auto err = strerror(errno);
	switch (eventType) {
	case READ:
		log(LOG, "Error while reading: {}", err);
	case WRITE:
		log(LOG, "Error while writing: {}", err);
	}
}

void CSession::onRecv(const SRecvData &data) {
	log(LOG, "Received: {}", data.data, data.dataSize);
}

void CSession::onSend(const std::string &msg) {
	log(LOG, "Sent: {}", msg);
}

void CSession::recvLoop() {
	while (true) {
		SRecvData recvData = read();
		if (!recvData.good)
			break;

		g_pChatManager->newMessage({.msg = recvData.data, .username = m_name});
	}
}

CSession::SRecvData CSession::read() {
	SRecvData recvData;
	ssize_t	  dataSize = recv(m_sockfd.get(), recvData.data, recvData.dataSize, 0);
	if (dataSize < 0) {
		recvData.good = false;
		onErrno(READ);
	}
	if (dataSize == 0) {
		recvData.good = false;
		// onDisconnect();
	}

	onRecv(recvData);
	return recvData;
}

CSession::SRecvData CSession::read(const std::string &msg) {
	write({}, "{}", msg, false);
	return read();
}

void CSession::run() {
	onConnect();
	registerSession() ? recvLoop() :
					  // TODO: get ip and print the ip also
		log(LOG, "Client {} exited without even registering", m_ip);

	g_pSessionManager->removeSession(self);
}

bool CSession::registerSession() {
	SRecvData recvData = read("Name: ");
	if (!recvData.good)
		return false;

	if (recvData.data[0] == '\n' || recvData.data[0] == ' ') {
		write("Name cannot be empty", eFormatType::ERR);
		return registerSession();
	}

	*std::remove(recvData.data, recvData.data + strlen(recvData.data), '\n') = '\0';
	if (g_pSessionManager->nameExists(recvData.data)) {
		write("Name already exists", eFormatType::ERR);
		return registerSession();
	}

	this->m_name = recvData.data;

	log(LOG, "Client registered as: {}", m_name);

	return true;
}

bool CSession::isValid() {
	if (!m_sockfd.isValid() || m_name.empty())
		return false;

	if (send(m_sockfd.get(), "", 0, MSG_NOSIGNAL) < 0)
		return false;

	return true;
}

template <typename... Args>
bool CSession::write(eFormatType type, std::format_string<Args...> fmt, Args &&...args) {
	std::string msg = NFormatter::fmt(type, fmt, std::forward<Args>(args)...);

	if (send(m_sockfd.get(), msg.c_str(), msg.size(), 0) < 0) {
		onErrno(WRITE);
		return false;
	}
	onSend(msg);
	return true;
}

bool CSession::write(const std::string &msg, eFormatType type) {
	return write(type, "{}", msg);
}

std::string CSession::getName() {
	return m_name;
}

void CSession::setSelf(std::pair<std::jthread, std::shared_ptr<CSession>> *self) {
	this->self = self;
}

void CSession::onShutdown() {
	write(SYS, "Shutting down, bye");
	g_pSessionManager->removeSession(self);
}
