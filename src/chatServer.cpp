#include "chatServer.hpp"
#include "server.hpp"
#include "sessionManager.hpp"
#include "chatManager.hpp"

CChatServer::CChatServer() = default;

CChatServer::~CChatServer() {
  cleanup();
}

void CChatServer::cleanup() {
  g_pChatManager.reset();
  g_pSessionManager.reset();
  g_pServer.reset();

  if (m_sessionThread.joinable())
    m_sessionThread.join();
}

void CChatServer::initManagers() {
  g_pServer		  = std::make_unique<CServer>(8080);
	g_pSessionManager = std::make_shared<CSessionManager>();
  g_pChatManager    = std::make_unique<CChatManager>();
}

void CChatServer::start() {
  initManagers();

  // will await while waiting for accept() to return a new client
  m_sessionThread = std::jthread(&CSessionManager::enterLoop, g_pSessionManager);
  m_sessionThread.detach();

  while (true) {
  }
}

