#include "chatManager.hpp"
#include "sessionManager.hpp"
#include "log.hpp"

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

void CChatManager::broadcastMessage(const SMessage &msg) {
  g_pSessionManager->broadcastChat(NFormatter::fmt(eFormatType::NONE, "{}: {}", msg.username, msg.msg), msg.username);
}
