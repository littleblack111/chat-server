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

void CInputManager::inputLoop() {
	while (true) {
		std::string input;
		std::getline(std::cin, input);

		if (input.empty())
			continue;

		g_pCommandHandler->isCommand(input) ? g_pCommandHandler->handleCommand(input) : g_pChatManager->newMessage({.msg = input, .username = "Server"});
	}
}
