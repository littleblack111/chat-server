#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

class CSession;

class CChatManager {
  public:
	CChatManager();
	~CChatManager();
	struct SMessage {
		std::string							   msg;
		std::string							   username;
		std::optional<std::weak_ptr<CSession>> sender = std::nullopt;
		bool								   admin  = false;
	};
	void		newMessage(const SMessage &msg);
	std::string fmtBroadcastMessage(const SMessage &msg);

	std::vector<SMessage> getChat() const;
};
inline std::unique_ptr<CChatManager> g_pChatManager;
