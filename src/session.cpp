#include "session.hpp"
#include "chatManager.hpp"
#include "log.hpp"
#include "server.hpp"
#include "sessionManager.hpp"
#include <arpa/inet.h>
#include <algorithm>
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
	log(LOG, "Received: {}", data.data);
}

void CSession::onSend(const std::string &msg) {
	log(LOG, "Sent: {}", msg);
}

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
		data = data.substr(start, end - start + 1);
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
		// onDisconnect();
	else
		recvData->data.resize(size);

	onRecv(*recvData);
	m_isReading = false;
	return recvData;
}

std::unique_ptr<CSession::SRecvData> CSession::read(const std::string &msg) {
	write(NONEWLINE, "{}", msg, false);
	m_isReading = true;
	return read();
}

void CSession::run() {
	onConnect();
	if (registerSession()) {
    // g_pSessionManager->broadcast(g_pChatManager->getChat());
    for (const auto &chat : g_pChatManager->getChat())
      write(g_pChatManager->fmtBroadcastMessage(chat));
    recvLoop();
  } else
    log(LOG, "Client {} exited without even registering", m_ip);

	g_pSessionManager->removeSession(self);
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

	if (send(m_sockfd.get(), "", 0, MSG_NOSIGNAL) < 0)
		return false;

	return true;
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
	onSend(msg);
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
	onSend(msg);
	return true;
}

bool CSession::write(const std::string &msg) {
	return write("{}", msg);
}

std::string CSession::getName() {
	return m_name;
}

void CSession::setSelf(std::pair<std::jthread, std::shared_ptr<CSession>> *self) {
	this->self = self;
}

void CSession::onShutdown() {
	write("Shutting down, bye");
	g_pSessionManager->removeSession(self);
}
