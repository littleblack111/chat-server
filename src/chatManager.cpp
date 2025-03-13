#include "chatManager.hpp"
#include "log.hpp"
#include "sessionManager.hpp"

CChatManager::CChatManager() {
	log(LOG, "ChatManager: initialized");
}

CChatManager::~CChatManager() {
	m_vMessages.clear();
	log(SYS, "ChatManager: bye");
}

void CChatManager::newMessage(const SMessage &msg) {
	log(LOG, "New message from {}: {}", msg.username, msg.msg);
	broadcastMessage(msg);
	m_vMessages.push_back(msg);
}

std::string CChatManager::fmtBroadcastMessage(const SMessage &msg) {
	return NFormatter::fmt(NONE, "{}: {}", msg.username, msg.msg);
}

void CChatManager::broadcastMessage(const SMessage &msg) {
	g_pSessionManager->broadcastChat(fmtBroadcastMessage(msg), msg.username);
}

std::vector<CChatManager::SMessage> CChatManager::getChat() {
	return m_vMessages;
}
