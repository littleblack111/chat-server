#include "chatServer.hpp"
#include "../io/IOManager.hpp"
#include "chatManager.hpp"
#include "../command/commandHandler.hpp"
#include "../command/commands.hpp"
#include "../io/inputManager.hpp"
#include "../debug/log.hpp"
#include "../io/renderManager.hpp"
#include "server.hpp"
#include "sessionManager.hpp"

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
	if (g_pSessionManager) {
		g_pSessionManager->shutdownSessions();
		g_pSessionManager.reset();
	}

	g_pChatManager.reset();
	g_pServer.reset();
	g_pCommandHandler.reset();
	g_pInputManager.reset();
	g_pIOManager.reset();
	g_pRenderManager.reset();
}

void CChatServer::initManagers() {
	log(LOG, "ChatServer: initializing managers");

	log(LOG, "Creating RenderManager");
	g_pRenderManager = std::make_unique<CRenderManager>();
	log(LOG, "Creating InputManager");
	g_pInputManager = std::make_unique<CInputManager>();
	log(LOG, "Creating Server");
	g_pServer = std::make_unique<CServer>(m_port);
	log(LOG, "Creating IOManager");
	g_pIOManager = std::make_unique<CIOManager>();
	log(LOG, "Creating SessionManager");
	g_pSessionManager = std::make_shared<CSessionManager>();
	log(LOG, "Creating ChatManager");
	g_pChatManager = std::make_unique<CChatManager>();
}

void CChatServer::start() {
	initManagers();

	log(LOG, "Creating CommandHandler");
	g_pCommandHandler = std::make_unique<CCommandHandler>();
	registerCommands();

	// will await while waiting for accept() to return a new client
	m_sessionManagerThread = std::jthread(&CSessionManager::run, g_pSessionManager.get());
	m_sessionManagerThread.detach();

	g_pInputManager->inputLoop();
}
