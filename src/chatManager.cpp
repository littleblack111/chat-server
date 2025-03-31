#include "chatManager.hpp"
#include "IOManager.hpp"
#include "log.hpp"
#include "sessionManager.hpp"

CChatManager::CChatManager() {
	log(LOG, "ChatManager: initialized");
}

CChatManager::~CChatManager() {
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
	g_pIOManager->m_vIO.push_back({.msg = msg, .log = std::nullopt});
}

std::string CChatManager::fmtBroadcastMessage(const SMessage &msg) {
	return NFormatter::fmt(NONE, "{}: {}", msg.username, msg.msg);
}

std::vector<CChatManager::SMessage> CChatManager::getChat() const {
	std::vector<SMessage> chat;

	for (const auto &[msg, log] : g_pIOManager->getIO())
		if (msg)
			chat.push_back(*msg);
	return chat;
}
