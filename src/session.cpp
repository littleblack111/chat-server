#include "session.hpp"
#include "chatManager.hpp"
#include "log.hpp"
#include "server.hpp"
#include "sessionManager.hpp"
#include <algorithm>
#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <sys/socket.h>

CSession::CSession()
	: m_addrLen(sizeof(m_addr)) {
	m_sockfd = Hyprutils::OS::CFileDescriptor{accept(g_pServer->getSocket()->get(), reinterpret_cast<sockaddr *>(&m_addr), &m_addrLen)}; // if this is in the init list, it will run before m_addrLen, so it won't work :/

	if (!m_sockfd.isValid())
		throw std::runtime_error("session: Failed to create socket");

	inet_ntop(AF_INET, &m_addr.sin_addr, m_ip, INET_ADDRSTRLEN);
	m_port = ntohs(m_addr.sin_port);
	log(LOG, "Client {} connected on port {}", m_ip, m_port);

	log(TRACE, "session: initialized");
}

CSession::~CSession() {
#ifdef DEBUG
	onDisconnect();
#endif
	m_sockfd.reset();

	log(LOG, "Client {}@{} disconnected", m_name, m_ip);

	log(TRACE, "session({}@{}): bye", m_name, m_ip);
}

#ifdef DEBUG
void CSession::onConnect() {
	log(LOG, "Client connected");
}

void CSession::onDisconnect() {
	log(LOG, "Client disconnected");
}
#endif

void CSession::onErrno(eEventType eventType) {
	const auto err = strerror(errno);
	switch (eventType) {
	case READ:
		log(WARN, "Error while reading: {}", err);
	case WRITE:
		log(WARN, "Error while writing: {}", err);
	}
}

#ifdef DEBUG
void CSession::onRecv(const SRecvData &data) {
	log(LOG, "Received: {}", data.data);
}

void CSession::onSend(const std::string &msg) {
	log(LOG, "Sent: {}", msg);
}
#endif

void CSession::recvLoop() {
	while (true) {
		auto recvData = read(NFormatter::fmt(NONEWLINE, "{}: ", m_name));
		if (!recvData->good)
			break;

		g_pChatManager->newMessage({.msg = recvData->data, .username = m_name});
	}
}

bool CSession::SRecvData::isEmpty() const {
	return data.empty() || std::ranges::all_of(data, [](char c) { return std::isspace(c); });
}

void CSession::SRecvData::sanitize() {
	if (!data.empty() && data.back() == '\n')
		data.pop_back();

	if (const auto start = data.find_first_not_of(" \t\r\n"); start != std::string::npos) {
		const auto end = data.find_last_not_of(" \t\r\n");
		data		   = data.substr(start, end - start + 1);
	} else
		data.clear();
}

std::unique_ptr<CSession::SRecvData> CSession::read() {
	auto recvData = std::make_unique<SRecvData>();

	recvData->data.resize(recvData->size);
	ssize_t size = recv(m_sockfd.get(), &recvData->data[0], recvData->size, 0);

	if (size < 0) {
		recvData->good = false;
		onErrno(READ);
	} else if (size == 0)
		recvData->good = false;
	else {
		recvData->data.resize(size);
    recvData->good = true;
  }

#ifdef DEBUG
	onRecv(*recvData);
#endif
	m_isReading = false;
	return recvData;
}

std::unique_ptr<CSession::SRecvData> CSession::read(const std::string &msg) {
	write(NONEWLINE, "{}", msg, false);
	m_isReading = true;
	return read();
}

void CSession::run() {
#ifdef DEBUG
	onConnect();
#endif
	if (registerSession()) {
		for (const auto &chat : g_pChatManager->getChat())
			write(g_pChatManager->fmtBroadcastMessage(chat));
		recvLoop();
	} else
		log(LOG, "Client {} exited without even registering", m_ip);

	g_pSessionManager->kick(self);
}

bool CSession::registerSession() {
	auto recvData = read("Name: ");
	if (!recvData->good)
		return false;

	recvData->sanitize();
	if (recvData->isEmpty()) {
		write("Name cannot be empty");
		return registerSession();
	}

	if (g_pSessionManager->nameExists(recvData->data)) {
		write("Name already exists");
		return registerSession();
	}

	this->m_name = recvData->data;

	log(LOG, "Client registered as: {}", m_name);

	return true;
}

bool CSession::isValid() {
	if (!m_sockfd.isValid() || m_name.empty())
		return false;

	int		  err  = 0;
	socklen_t size = sizeof(err);
	return getsockopt(m_sockfd.get(), SOL_SOCKET, SO_ERROR, &err, &size) >= 0 && err == 0;
}

template <typename... Args>
bool CSession::write(std::format_string<Args...> fmt, Args &&...args) {
	std::string msg = NFormatter::fmt(NONE, fmt, std::forward<Args>(args)...);
	if (m_isReading)
		msg.insert(0, "\n");

	if (send(m_sockfd.get(), msg.c_str(), msg.size(), 0) < 0) {
		onErrno(WRITE);
		return false;
	}
#ifdef DEBUG
	onSend(msg);
#endif
	return true;
}

template <typename... Args>
bool CSession::write(eFormatType type, std::format_string<Args...> fmt, Args &&...args) {
	std::string msg = NFormatter::fmt(type, fmt, std::forward<Args>(args)...);
	if (m_isReading)
		msg.insert(0, "\n");

	if (send(m_sockfd.get(), msg.c_str(), msg.size(), 0) < 0) {
		onErrno(WRITE);
		return false;
	}
#ifdef DEBUG
	onSend(msg);
#endif
	return true;
}

bool CSession::write(const std::string &msg) {
	return write("{}", msg);
}

const std::string &CSession::getName() const {
	return m_name;
}

void CSession::setSelf(std::pair<std::jthread, std::shared_ptr<CSession>> *self) {
	this->self = self;
}
