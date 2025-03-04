#include "sessionManager.hpp"
#include "session.hpp"
#include "server.hpp"
#include <print>

CSessionManager::CSessionManager() = default;

CSessionManager::~CSessionManager() {
  for (auto &[thread, session] : m_vSessions) {
    thread.join();
  }
}

void CSessionManager::newSession(Hyprutils::OS::CFileDescriptor sockfd) {
  std::shared_ptr session = std::make_shared<CSession>(std::move(sockfd));
  m_vSessions.emplace_back(std::jthread(&CSession::run, session), std::move(session));
}

void CSessionManager::enterLoop() {
  while (true) {
    newSession(Hyprutils::OS::CFileDescriptor{accept(g_pServer->getSocket().take(), nullptr, nullptr) /* awaits new client/session */});
  }
}

void CSessionManager::broadcast(const std::string &msg) const {
  for (const auto &[thread, session] : m_vSessions) {
    session->write(msg);
  }
}
