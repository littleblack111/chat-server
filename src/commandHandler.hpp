#include <functional>
#include <memory>
#include <string>
#include <vector>

class CCommandHandler {
  public:
	CCommandHandler();
	~CCommandHandler();

	struct SCommand {
		struct SResult {
			std::string result;
			bool		good;
		};

		std::string										name;
		std::function<SResult(const std::string &args)> parser;
		std::function<SResult(const std::string &args)> exe;
	};

	bool isCommand(const std::string &str) const;
	bool validCommand(const std::string &command) const;
	// return false if command struct is invalid
	bool							   registerCommand(const SCommand &command);
	CCommandHandler::SCommand::SResult newCommand(const std::string &command, const std::string &args) const;

  private:
	const CCommandHandler::SCommand *getCommand(const std::string &command) const;
	CCommandHandler::SCommand::SResult									exeCommand(const SCommand &command, const std::string &args) const;

	std::vector<SCommand> m_vCommands;
};
inline std::unique_ptr<CCommandHandler> g_pCommandHandler;
