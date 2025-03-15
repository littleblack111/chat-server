#include "commands.hpp"
#include "commandHandler.hpp"

void registerCommands() {
	g_pCommandHandler->registerCommand({
		.command = "test",
		.parser	 = [](const std::string &args) -> CCommandHandler::SCommand::SResult {
			 return {.result = args, .good = true};
		},
		.exe = [](const std::string &args) -> CCommandHandler::SCommand::SResult {
			return {.result = args, .good = true};
		},

	});
}
