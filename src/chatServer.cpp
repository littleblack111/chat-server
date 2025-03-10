#include "chatServer.hpp"
#include "server.hpp"
#include "sessionManager.hpp"
#include "chatManager.hpp"
#include "log.hpp"

CChatServer::CChatServer() {
  log(LOG, "ChatServer: initialized");
};

CChatServer::~CChatServer() {
  cleanup();
  log(SYS, "ChatServer: bye");
}

void CChatServer::cleanup() {
  g_pChatManager.reset();
  // if (m_sessionThread.joinable())
    // m_sessionThread.join();
  g_pSessionManager.reset();
  g_pServer.reset();
}

void CChatServer::initManagers() {
  log(LOG, "ChatServer: initializing managers");
  log(LOG, "Creating Server");
  g_pServer		  = std::make_unique<CServer>(8080);
  log(LOG, "Creating SessionManager");
	g_pSessionManager = std::make_shared<CSessionManager>();
  log(LOG, "Creating ChatManager");
  g_pChatManager    = std::make_unique<CChatManager>();
}

void CChatServer::start() {
  initManagers();

  // will await while waiting for accept() to return a new client
  m_sessionThread = std::jthread(&CSessionManager::run, g_pSessionManager.get());
  m_sessionThread.detach();

  while (true) {
  }
}

