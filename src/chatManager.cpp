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
	log(LOG, "{}: {}", msg.username, msg.msg);
	const auto session = g_pSessionManager->getByName(msg.username);
	if (session && session->isMuted()) {
		session->write("You are muted");
		return;
	}
	g_pSessionManager->broadcastChat(msg);
	m_vMessages.push_back(msg);
}

std::string CChatManager::fmtBroadcastMessage(const SMessage &msg) {
	return NFormatter::fmt(NONE, "{}: {}", msg.username, msg.msg);
}

const std::vector<CChatManager::SMessage> &CChatManager::getChat() const {
	return m_vMessages;
}
