#include "commandHandler.hpp"
#include "log.hpp"
#include <algorithm>

CCommandHandler::CCommandHandler() {
	log(LOG, "CommandHandler: initialized");
}

CCommandHandler::~CCommandHandler() {
	log(SYS, "CommandHandler: bye");
	m_vCommands.clear();
}

bool CCommandHandler::isCommand(const std::string &str) const {
	return !str.empty() && str.front() == '/';
};

bool CCommandHandler::validCommand(const std::string &command) const {
	return std::ranges::any_of(m_vCommands, [&command](const SCommand &cmd) {
		return cmd.name == command;
	});
}

bool CCommandHandler::registerCommand(const SCommand &command) {
	if (command.name.empty() || !command.exe || !command.parser)
		return false;

	m_vCommands.push_back(command);
	return true;
}

const CCommandHandler::SCommand *CCommandHandler::getCommand(const std::string &command) const {
	auto it = std::ranges::find_if(m_vCommands, [&command](const SCommand &cmd) { return cmd.name == command; });

	if (it != m_vCommands.end())
		return &*it;

	return nullptr;
}

CCommandHandler::SResult CCommandHandler::exeCommand(const SCommand &command, const std::string &args) const {
	const auto [parsed, good] = command.parser(args);
	if (!good)
		try {
			return {.result = std::any_cast<std::string>(parsed), .good = false};
		} catch (const std::bad_any_cast &) {
			return {.result = "", .good = false};
		}

	return command.exe(parsed);
}

CCommandHandler::SResult CCommandHandler::newCommand(const std::string &command, const std::string &args) const {
	if (!validCommand(command))
		return {.result = "Invalid command", .good = false};

	return exeCommand(*getCommand(command), args);
}
