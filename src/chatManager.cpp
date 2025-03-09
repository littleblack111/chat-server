#include "chatManager.hpp"
#include "sessionManager.hpp"

CChatManager::~CChatManager() {
  m_vMessages.clear();
}

void CChatManager::newMessage(const SMessage &msg) {
  broadcastMessage(msg);
  m_vMessages.push_back(msg);
}

void CChatManager::broadcastMessage(const SMessage &msg) {
  g_pSessionManager->broadcastChat(NFormatter::fmt(eFormatType::NONE, "{}: {}", msg.username, msg.msg), msg.username);
}
