#include "session.hpp"
#include <cmath>
#include <ostream>
#include <sys/socket.h>
#include <print>
#include <cerrno>
#include <cstring>

CSession::CSession(Hyprutils::OS::CFileDescriptor sockfd) : m_sockfd(std::move(sockfd)) {
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

void CSession::onErrno() const {
  std::println("Error: {}", strerror(errno));
}

void CSession::onRecv(SRecvData &data) const {
  std::println("Received: {}", data.data, data.dataSize);
}

void CSession::recvManager() {
  while (true) {
    CSession::SRecvData recvData = read();
    if (!recvData.good) {
      break;
    }
  }
}

CSession::SRecvData CSession::read() {
  SRecvData recvData;
  ssize_t dataSize = recv(m_sockfd.get(), recvData.data, recvData.dataSize, 0);
  if (dataSize < 0) {
    recvData.good = false;
    onErrno();
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
  SRecvData recvData;
  ssize_t dataSize = recv(m_sockfd.get(), recvData.data, recvData.dataSize, 0);

  if (dataSize > 0) {
    recvData.good = false;

    if (dataSize < 0)
      onErrno();

    if (dataSize == 0)
      onDisconnect();
  }
  onRecv(recvData);

  return recvData;
}

void CSession::run() {
  onConnect();
  // std::jthread recvThread(&CSession::recvManager, this);
  registerSession();
  recvManager();
}

bool CSession::registerSession() {
  SRecvData rervData = read((std::string&)"Name: ");
  if (!rervData.good) {
    return false;
  }
  m_name = rervData.data;

  std::println("Client registered as: {}", m_name);

  return true;
}

bool CSession::isValid() const {
  return m_sockfd.isValid() && !m_name.empty();
}

bool CSession::write(const std::string &msg) const {
  bool bad = false;
  ssize_t bytes_sent = send(m_sockfd.get(), msg.c_str(), msg.size(), 0);
  if (bytes_sent < 0) {
    bad = true;
    onErrno();
  }
  return bad;
}

std::string CSession::getName() const {
  return m_name;
}
