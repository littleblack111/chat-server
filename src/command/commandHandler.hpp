#pragma once

#include "../core/session.hpp"
#include <any>
#include <functional>
#include "../misc/memory.hpp"
#include <netinet/in.h>
#include <string>
#include <vector>

class CCommandHandler {
  public:
	CCommandHandler();
	~CCommandHandler();

	struct SResult {
		std::string result;
		bool		good = false;
	};

	struct SParseResult {
		std::any data;
		bool	 good = false;
	};

	struct SCommand {
		bool												 requireAdmin = true;
		std::string											 name;
		std::function<SParseResult(const std::string &args)> parser;
		std::function<SResult(const std::any &parsed)>		 exe;
	};

	bool isCommand(const std::string &str) const;
	bool validCommand(const std::string &command) const;
	// return false if command struct is invalid
	bool	registerCommand(const SCommand &command);
	SResult newCommand(const std::string &command, const std::string &args, SP<CSession> session) const;
	void	handleCommand(std::string input, SP<CSession> user = nullptr) const;

	template <typename T>
	struct SMakeCommandArgs {
		std::string												   name;
		std::function<std::pair<T, bool>(const std::string &args)> parser;
		std::function<SResult(const T &parsed)>					   exe;
	};

	template <typename T>
	static SCommand makeCommand(SMakeCommandArgs<T> args) {
		return {
			.name	= std::move(args.name),
			.parser = [parser = std::move(args.parser)](const std::string &args) -> SParseResult {
				auto [data, good] = parser(args);
				return {.data = std::any(std::move(data)), .good = good};
			},
			.exe = [exe = std::move(args.exe)](const std::any &parsed) -> SResult {
				try {
					return exe(std::any_cast<const T &>(parsed));
				} catch (const std::bad_any_cast &) {
					return {.result = "Invalid data type", .good = false};
				}
			}
		};
	}

  private:
	SP<CCommandHandler::SCommand> getCommand(const std::string &command) const;
	SResult									   exeCommand(const SP<SCommand> command, const std::string &args) const;

	std::vector<SP<SCommand>> m_vCommands;
};
inline UP<CCommandHandler> g_pCommandHandler;
