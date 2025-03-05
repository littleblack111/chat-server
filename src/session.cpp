#include "session.hpp"
#include "log.hpp"
#include <cerrno>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>

CSession::CSession(Hyprutils::OS::CFileDescriptor sockfd)
	: m_sockfd(std::move(sockfd)) {
	if (!m_sockfd.isValid())
		throw std::runtime_error("Failed to create socket");
}

CSession::~CSession() {
	onDisconnect();
	m_sockfd.reset();
}

void CSession::onConnect() const {
	log(INFO, "Client connected");
}

void CSession::onDisconnect() const {
	log(INFO, "Client disconnected");
}

void CSession::onErrno(eEventType eventType) const {
	const auto err = strerror(errno);
	switch (eventType) {
	case READ:
		log(ERROR, "Error while reading: {}", err);
	case WRITE:
		log(ERROR, "Error while writing: {}", err);
	}
}

void CSession::onRecv(const SRecvData &data) const {
	log(INFO, "Received: {}", data.data, data.dataSize);
}

void CSession::onSend(const std::string &msg) const {
	log(INFO, "Sent: {}", msg);
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

CSession::SRecvData CSession::read(const std::string &msg) {
	write(eFormatType::STANDARD, "{}", msg, false);
	return read();
}

void CSession::run() {
	onConnect();
	registerSession();
	recvManager();
}

bool CSession::registerSession() {
	SRecvData recvData = read("Name: ");
	if (!recvData.good)
		return false;

	if (recvData.data[0] == '\n' || recvData.data[0] == ' ') {
		write("Name cannot be empty", eFormatType::ERROR);
		registerSession();
	}

	m_name = recvData.data;

	std::println("Client registered as: {}", m_name);

	return true;
}

bool CSession::isValid() const {
	if (!m_sockfd.isValid() || m_name.empty())
		return false;

	if (send(m_sockfd.get(), "", 0, MSG_NOSIGNAL) < 0)
		return false;

	return true;
}

template <typename... Args>
bool CSession::write(eFormatType type, std::format_string<Args...> fmt, Args &&...args) const {
	std::string msg = CFormatter::fmt(type, fmt, std::forward<Args>(args)...);

	if (send(m_sockfd.get(), msg.c_str(), msg.size(), 0) < 0) {
		onErrno(WRITE);
		return false;
	}
	onSend(msg);
	return true;
}

bool CSession::write(const std::string &msg, eFormatType type) const {
	return write(type, "{}", msg);
}

std::string CSession::getName() const {
	return m_name;
}
