#pragma once

#include <memory>
#include <string>
#include <vector>

class CChatManager {
  public:
	CChatManager();
	~CChatManager();
	struct SMessage {
		std::string msg;
		std::string username;
	};
	void		newMessage(const SMessage &msg);
	void		broadcastMessage(const SMessage &msg);
	std::string fmtBroadcastMessage(const SMessage &msg);

	const std::vector<SMessage> &getChat() const;

  private:
	std::vector<SMessage> m_vMessages;
};
inline std::unique_ptr<CChatManager> g_pChatManager;
