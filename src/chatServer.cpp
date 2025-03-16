#include "chatServer.hpp"
#include "chatManager.hpp"
#include "log.hpp"
#include "server.hpp"
#include "sessionManager.hpp"
#include <unistd.h>

CChatServer::CChatServer(uint16_t port)
	: m_port(port) {
	if (std::atexit([]() { g_pChatServer->cleanup(); }))
		log(ERR, "Failed to register CChatServer atexit handler, dtors will not be called");

	log(LOG, "ChatServer: initialized");
};

CChatServer::~CChatServer() {
	cleanup();
	log(SYS, "ChatServer: bye");
}

void CChatServer::cleanup() {
	g_pChatManager.reset();
	// if (m_sessionManagerThread.joinable())
	// m_sessionManagerThread.join();
	g_pSessionManager.reset();
	g_pServer.reset();
}

void CChatServer::initManagers() {
	log(LOG, "ChatServer: initializing managers");
	log(LOG, "Creating Server");
	g_pServer = std::make_unique<CServer>(m_port);
	log(LOG, "Creating SessionManager");
	g_pSessionManager = std::make_shared<CSessionManager>();
	log(LOG, "Creating ChatManager");
	g_pChatManager = std::make_unique<CChatManager>();
}

void CChatServer::start() {
	initManagers();

	// will await while waiting for accept() to return a new client
	m_sessionManagerThread = std::jthread(&CSessionManager::run, g_pSessionManager);
	m_sessionManagerThread.detach();

	// while (true) {
	// }
  sleep(3);
  g_pSessionManager->kick(g_pSessionManager->getByName("asd"));
  // g_pSessionManager->getByName("asd");
	while (true) {
	}
}
