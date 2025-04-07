#pragma once

#include "chatManager.hpp"
#include "format.hpp"
#include <mutex>
#include <optional>

class CIOManager {
  public:
	CIOManager();
	~CIOManager();

	struct SLog {
		eFormatType type;
		std::string log;
	};
	struct SIO {
		// cannot have both at once
		std::optional<CChatManager::SMessage> msg;
		std::optional<SLog>					  log;
	};

	const std::vector<SIO> getIO() const;
	void				   addLog(const SLog &log);

  private:
	mutable std::mutex m_mutex;
	std::vector<SIO>   m_vIO;

	friend class CChatManager;
};
inline std::unique_ptr<CIOManager> g_pIOManager;
