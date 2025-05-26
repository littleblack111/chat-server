#pragma once

#include "chatManager.hpp"
#include "format.hpp"
#include <mutex>
#include <optional>
#include <vector>

class CIOManager {
  public:
	CIOManager();
	~CIOManager();

	struct SLog {
		eFormatType type;
		std::string log;
	};
	struct SIO {
		// once a time
		std::optional<CChatManager::SMessage>			   msg	  = std::nullopt;
		std::optional<SLog>								   log	  = std::nullopt;
		std::optional<std::pair<std::string, std::string>> custom = std::nullopt;
	};

	const std::vector<SIO> &getIO() const;
	void					addLog(const SLog &log);
	void					addMessage(const CChatManager::SMessage &msg);
	void					addCustom(const std::pair<std::string, std::string> &custom);

  private:
	std::vector<SIO> m_vIO;

	mutable std::mutex m_mutex;

	friend class CChatManager;
};
inline std::unique_ptr<CIOManager> g_pIOManager;
