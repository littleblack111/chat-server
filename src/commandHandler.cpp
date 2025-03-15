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
	return str.front() == '/';
};

bool CCommandHandler::validCommand(const std::string &command) const {
	return std::ranges::any_of(m_vCommands, [&command](const SCommand &cmd) {
		return cmd.command == command;
	});
}

bool CCommandHandler::registerCommand(const SCommand &command) {
	if (command.command.empty() || !command.exe || !command.parser)
		return false;

	m_vCommands.push_back(command);
	return true;
}

std::function<CCommandHandler::SCommand::SResult(std::string args)> CCommandHandler::getCommand(const std::string &command) const {
	return std::ranges::find_if(m_vCommands, [&command](const SCommand &cmd) {
			   return cmd.command == command;
		   })
		->exe;
}

std::function<CCommandHandler::SCommand::SResult(const std::string &args)> CCommandHandler::getParser(const std::string &command) const {
	return std::ranges::find_if(m_vCommands, [&command](const SCommand &cmd) {
			   return cmd.command == command;
		   })
		->parser;
}

CCommandHandler::SCommand::SResult CCommandHandler::exeCommand(const std::string &command, const std::string &args) const {
	const auto parser = getParser(command)(args);
	if (!parser.good)
		return {.result = {}, .good = false};

	return getCommand(command)(parser.result);
}

CCommandHandler::SCommand::SResult CCommandHandler::newCommand(const std::string &command, const std::string &args) const {
	if (!validCommand(command))
		return {.result = {}, .good = false};

	return exeCommand(command, args);
}
