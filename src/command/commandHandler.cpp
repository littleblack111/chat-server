#include "commandHandler.hpp"
#include "../debug/log.hpp"
#include "../core/session.hpp"
#include <algorithm>
#include <memory>

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
	return std::ranges::any_of(m_vCommands, [&command](const std::shared_ptr<SCommand> &cmd) {
		return cmd->name == command;
	});
}

bool CCommandHandler::registerCommand(const SCommand &command) {
	if (command.name.empty() || !command.exe || !command.parser)
		return false;

	m_vCommands.push_back(std::make_shared<SCommand>(command));

	log(TRACE, "CommandHandler: registered command: {}", command.name);

	return true;
}

std::shared_ptr<CCommandHandler::SCommand> CCommandHandler::getCommand(const std::string &command) const {
	auto it = std::ranges::find_if(m_vCommands, [&command](const std::shared_ptr<SCommand> &cmd) { return cmd->name == command; });

	if (it != m_vCommands.end())
		return *it;

	return nullptr;
}

CCommandHandler::SResult CCommandHandler::exeCommand(const std::shared_ptr<SCommand> command, const std::string &args) const {
	const auto [parsed, good] = command->parser(args);
	if (!good)
		try {
			return {.result = std::any_cast<std::string>(parsed), .good = false};
		} catch (const std::bad_any_cast &) {
			return {.result = "", .good = false};
		}

	log(TRACE, "CommandHandler: executing command: {}", command->name);
	return command->exe(parsed);
}

CCommandHandler::SResult CCommandHandler::newCommand(const std::string &command, const std::string &args, std::shared_ptr<CSession> session) const {
	if (!validCommand(command))
		return {.result = "Invalid command", .good = false};

	auto cmd = getCommand(command);
	if (!cmd)
		return {.result = "Command not found", .good = false};

	if (cmd->requireAdmin && !session->isAdmin())
		return {.result = "Permission denied", .good = false};

	return exeCommand(cmd, args);
}

void CCommandHandler::handleCommand(std::string input, std::shared_ptr<CSession> user) const {
	input = input.substr(1);

	std::string command;
	std::string args;

	size_t spacePos = input.find(' ');
	if (spacePos == std::string::npos) {
		command = input;
		args	= "";
	} else {
		command = input.substr(0, spacePos);
		// skip extra spaces
		size_t startPos = input.find_first_not_of(' ', spacePos);
		if (startPos != std::string::npos)
			args = input.substr(startPos);
	}

	args.empty() ? log(LOG, "Spawning Command: {}", command) : log(LOG, "Spawning Command: {}, Args: {}", command, args);
	const auto exe = newCommand(command, args, user);

	std::function<void(eFormatType, const std::string &)> outputter = [](eFormatType type, const std::string &msg) { log(type, "{}", msg); };

	if (!user->getIp().empty())
		outputter = [user](eFormatType, const std::string &msg) { user->write(msg); };

	exe.good ? exe.result.empty() ? outputter(LOG, "Command succeeded") : outputter(LOG, exe.result) : exe.result.empty() ? outputter(ERR, "Command failed")
																														  : outputter(ERR, exe.result);
};
