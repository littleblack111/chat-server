#include "session.hpp"
#include "chatManager.hpp"
#include "commandHandler.hpp"
#include "format.hpp"
#include "log.hpp"
#include "server.hpp"
#include "sessionManager.hpp"
#include <algorithm>
#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <sys/socket.h>

CSession::CSession() {
	m_sockfd = Hyprutils::OS::CFileDescriptor{accept(g_pServer->getSocket()->get(), reinterpret_cast<sockaddr *>(&m_addr), &m_addrLen)}; // if this is in the init list, it will run before m_addrLen, so it won't work :/

	if (!m_sockfd.isValid())
		throw std::runtime_error("session: Failed to create socket");

	m_ip.resize(INET_ADDRSTRLEN);
	inet_ntop(AF_INET, &m_addr.sin_addr, &m_ip[0], INET_ADDRSTRLEN);
	m_ip.resize(strlen(m_ip.c_str()));
	m_port	  = ntohs(m_addr.sin_port);
	m_isAdmin = std::ranges::any_of(m_adminIps, [this](const char *ip) { return m_ip == ip; });

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
	log(TRACE, "Client {}@{} connected", m_name, m_ip);
}

void CSession::onDisconnect() {
	log(TRACE, "Client {}@{} disconnected", m_name, m_ip);
}
#endif

void CSession::onErrno(eEventType eventType) {
	char		errbuf[256];
	const char *err = strerror_r(errno, errbuf, sizeof(errbuf));

	switch (eventType) {
	case READ:
		log(WARN, "Error while reading: {}", err);
		break;
	case WRITE:
		log(WARN, "Error while writing: {}", err);
		break;
	}
}

#ifdef DEBUG
void CSession::onRecv(const SRecvData &data) {
	log(TRACE, "Received: {}", data.data);
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

		recvData->sanitize();
		if (recvData->isEmpty()) {
			log(TRACE, "Empty message from {}", m_name);
			continue;
		}

		const auto isCommand = g_pCommandHandler->isCommand(recvData->data);
		if (isCommand)
			g_pCommandHandler->handleCommand(recvData->data, this);

		g_pChatManager->newMessage({.msg = recvData->data, .username = m_name, .sender = (uintptr_t)this, .admin = isCommand});
	}
}

bool CSession::SRecvData::isEmpty() const {
	return data.empty() || std::ranges::all_of(data, [](char c) { return std::isspace(c); });
}

void CSession::SRecvData::sanitize() {
	if (!data.empty() && data.back() == '\n')
		data.pop_back();

	data.erase(std::remove(data.begin(), data.end(), asciiEscape), data.end()); // don't accept ASCII code, might mess up terminal

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
	m_szReading.reset();
	return recvData;
}

std::unique_ptr<CSession::SRecvData> CSession::read(const std::string &msg, bool bypassDeaf) {
	const auto deaf = isDeaf();
	if (bypassDeaf && deaf)
		setDeaf(false);
	write(NONEWLINE, "{}", msg, false);
	if (bypassDeaf && deaf)
		setDeaf(true);
	m_szReading = msg;
	return read();
}

void CSession::run() {
#ifdef DEBUG
	onConnect();
#endif
	if (registerSession()) {
		for (const auto &chat : g_pChatManager->getChat())
			writeChat(chat);
		recvLoop();
	} else
		log(LOG, "Client {} exited without even registering", m_ip);

	g_pSessionManager->kick(self);
}

bool CSession::isMuted() const {
	return m_bMuted;
}

void CSession::setMuted(bool mute) {
	m_bMuted = mute;
}

bool CSession::isDeaf() const {
	return m_bDeaf;
}

void CSession::setDeaf(bool deaf) {
	m_bDeaf = deaf;
}

bool CSession::registerSession() {
	auto recvData = read("Name: ", true);
	if (!recvData->good)
		return false;

	recvData->sanitize();
	if (recvData->isEmpty()) {
		setDeaf(false);
		write("Name cannot be empty");
		setDeaf(true);
		return registerSession();
	}

	if (g_pSessionManager->nameExists(recvData->data)) {
		setDeaf(false);
		write("Name already exists");
		setDeaf(true);
		return registerSession();
	}

	this->m_name = recvData->data;

	log(LOG, "Client {} registered as: {}", m_ip, m_name);

	g_pSessionManager->broadcast(NFormatter::fmt(NONEWLINE, "{} has joined the chat", m_name), this);

	setDeaf(false);
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
	if (isDeaf())
		return false;
	return write(NONE, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
bool CSession::write(eFormatType type, std::format_string<Args...> fmt, Args &&...args) {
	if (isDeaf())
		return false;

	std::string msg = NFormatter::fmt(type, fmt, std::forward<Args>(args)...);
	if (m_szReading) {
		msg.insert(0, "\r");
		msg.append(*m_szReading);
	}

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

bool CSession::writeChat(const CChatManager::SMessage &msg) {
	if (msg.sender ? (uintptr_t)this != *msg.sender : msg.username != m_name) {
		if (msg.admin && !m_isAdmin)
			return false;

		return write(g_pChatManager->fmtBroadcastMessage(msg));
	}
	return false;
}

const std::string &CSession::getName() const {
	return m_name;
}

const std::string &CSession::getIp() const {
	return m_ip;
}

bool CSession::isAdmin() const {
	return m_isAdmin;
}

void CSession::setSelf(std::pair<std::jthread, std::shared_ptr<CSession>> *self) {
	this->self = self;
}
