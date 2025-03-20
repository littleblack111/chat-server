#include "commands.hpp"
#include "commandHandler.hpp"
#include "sessionManager.hpp"

struct SKickArgs {
	std::string target;
	std::string reason;
};

void registerCommands() {
	g_pCommandHandler->registerCommand({.name = "raw", .parser = [](const std::string &args) -> CCommandHandler::SParseResult { return {.data = args, .good = true}; }, .exe = [](const std::any &parsed) -> CCommandHandler::SResult {
      const std::string &data = std::any_cast<const std::string &>(parsed);
      g_pSessionManager->broadcast(data);
      return {.result = "Broadcasted: " + data, .good = true}; }});

	g_pCommandHandler->registerCommand(CCommandHandler::makeCommand<SKickArgs>({.name = "kick", .parser = [](const std::string &args) -> std::pair<SKickArgs, bool> {
      size_t pos = args.find(' ');
      std::string target = args;
      std::string reason = "No reason specified";

      if (pos != std::string::npos) {
        target = args.substr(0, pos);
        reason = args.substr(pos + 1);
        if (reason.empty())
          reason = "Kicked: No reason specified";
      }

      if (target.empty())
        return {SKickArgs{}, false};

      return {
        SKickArgs{.target=target, .reason=reason}, true}; }, .exe = [](const SKickArgs &data) -> CCommandHandler::SResult {
          const auto session = g_pSessionManager->getByName(data.target);
          if (!session)
            return {.result = "User '" + data.target + "' not found", .good = false};

          g_pSessionManager->kick(session, true, data.reason);
          return {.result = "Kicked '" + data.target + "': " + data.reason, .good = true}; }}));

	g_pCommandHandler->registerCommand({.name = "list", .parser = [](const std::string &args) -> CCommandHandler::SParseResult { return {.good = true}; }, .exe = [](const std::any &parsed) -> CCommandHandler::SResult {
      std::string result = "Clients:\n";
      for (const auto &session : g_pSessionManager->getSessions())
        result += session->getName() + "\t" + session->getIp() + "\n";

      return {.result = result, .good = true}; }});
}
