#include "inputManager.hpp"
#include "chatManager.hpp"
#include "commandHandler.hpp"
#include "log.hpp"
#include <iostream>

CInputManager::CInputManager() {
	log(LOG, "InputManager: initialized");
}

CInputManager::~CInputManager() {
	log(SYS, "InputManager: bye");
}

void CInputManager::run() {
	inputLoop();
}

static void handleCommand(std::string &input) {
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

	args.empty() ? log(TRACE, "Spawning Command: {}", command) : log(TRACE, "Spawning Command: {}, Args: {}", command, args);
	const auto exe = g_pCommandHandler->newCommand(command, args);
	exe.good ? exe.result.empty() ? log(LOG, "Command {} succeeded", command) : log(LOG, "Command {} succeeded: {}", command, exe.result) : log(ERR, "Command {} failed", command);
};

void CInputManager::inputLoop() {
	while (true) {
		std::string input;
		std::getline(std::cin, input);

		if (input.empty())
			continue;

		g_pCommandHandler->isCommand(input) ? handleCommand(input) : g_pChatManager->broadcastMessage({.msg = input, .username = "Server"});
	}
}
