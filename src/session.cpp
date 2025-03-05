#include "session.hpp"
#include <cerrno>
#include <cstring>
#include <print>
#include <sys/socket.h>

CSession::CSession(Hyprutils::OS::CFileDescriptor sockfd)
	: m_sockfd(std::move(sockfd)) {
	if (!m_sockfd.isValid())
		throw std::runtime_error("Failed to create socket");
}

CSession::~CSession() {
	m_sockfd.reset();
}

void CSession::onConnect() const {
	std::println("Client connected");
}

void CSession::onDisconnect() const {
	std::println("Client disconnected");
}

void CSession::onErrno(eEventType eventType) const {
	const auto err = strerror(errno);
	switch (eventType) {
	case READ:
		std::println("Error while reading: {}", err);
	case WRITE:
		std::println("Error while writing: {}", err);
	}
}

void CSession::onRecv(SRecvData &data) const {
	std::println("Received: {}", data.data, data.dataSize);
}

void CSession::recvManager() {
	while (true) {
		CSession::SRecvData recvData = read();
		if (!recvData.good)
			break;
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
		onDisconnect();
	}
	onRecv(recvData);
	return recvData;
}

CSession::SRecvData CSession::read(std::string &msg) {
	write(msg);
	return read();
}

void CSession::run() {
	onConnect();
	// std::jthread recvThread(&CSession::recvManager, this);
	registerSession();
	recvManager();
}

bool CSession::registerSession() {
	SRecvData recvData = read((std::string &)"Name: ");
	if (!recvData.good)
		return false;
	m_name = recvData.data;

	std::println("Client registered as: {}", m_name);

	return true;
}

bool CSession::isValid() const {
	return m_sockfd.isValid() && !m_name.empty();
}

bool CSession::write(const std::string &msg) const {
	bool	bad		 = false;
	ssize_t sentSize = send(m_sockfd.get(), msg.c_str(), msg.size(), 0);
	if (sentSize < 0) {
		bad = true;
		onErrno(WRITE);
	}
	return bad;
}

std::string CSession::getName() const {
	return m_name;
}
