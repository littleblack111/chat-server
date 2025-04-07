#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <vector>

class CChatManager {
  public:
	CChatManager();
	~CChatManager();
	struct SMessage {
		std::string msg;
		std::string username;
		bool		admin = false;
	};
	void		newMessage(const SMessage &msg);
	std::string fmtBroadcastMessage(const SMessage &msg);

	std::vector<SMessage> getChat() const;

  private:
	mutable std::mutex m_mutex;
};
inline std::unique_ptr<CChatManager> g_pChatManager;
