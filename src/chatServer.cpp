#include "chatServer.hpp"
#include "server.hpp"
#include "sessionManager.hpp"

CChatServer::CChatServer() = default;

CChatServer::~CChatServer() {
  cleanup();
}

void CChatServer::cleanup() {
  g_pServer.reset();
  g_pSessionManager.reset();
}

void CChatServer::initManagers() {
  g_pServer		  = std::make_unique<CServer>(8080);
	g_pSessionManager = std::make_shared<CSessionManager>();
}


void CChatServer::start() {
  initManagers();

  // will await while waiting for accept() to return a new client
  std::jthread sessionThread = std::jthread(&CSessionManager::enterLoop, g_pSessionManager);
  sessionThread.detach();

  while (true) {
  }
}

