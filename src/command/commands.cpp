#include "commands.hpp"
#include "commandHandler.hpp"
#include "../debug/format.hpp"
#include "../debug/log.hpp"
#include "../core/sessionManager.hpp"

#define NOREASON "No reason specified";

struct SKickArgs {
	std::string				   target;
	std::optional<std::string> reason;
};

struct SMuteArgs {
	std::string				   target;
	std::optional<std::string> reason;
};

void registerCommands() {
	g_pCommandHandler->registerCommand({.name = "raw", .parser = [](const std::string &args) -> CCommandHandler::SParseResult { return {.data = args, .good = true}; }, .exe = [](const std::any &parsed) -> CCommandHandler::SResult {
        const std::string &data = std::any_cast<const std::string &>(parsed);
        g_pSessionManager->broadcast(data);
        return {.result = "Broadcasted: " + data, .good = true}; }});

	g_pCommandHandler->registerCommand(CCommandHandler::makeCommand<SKickArgs>({.name = "kick", .parser = [](const std::string &args) -> std::pair<SKickArgs, bool> {
        size_t pos = args.find(' ');
        std::string target = args;
		std::optional<std::string> reason = std::nullopt;

        if (pos != std::string::npos) {
            target = args.substr(0, pos);
            reason = args.substr(pos + 1);
        }

        if (target.empty())
            return {SKickArgs{}, false};

        return {
            SKickArgs{.target=target, .reason=reason}, true}; }, .exe = [](const SKickArgs &data) -> CCommandHandler::SResult {
        const auto session = g_pSessionManager->getByName(data.target);
        if (!session)
            return {.result = "User '" + data.target + "' not found", .good = false};

        const auto reason = data.reason ? *data.reason : NOREASON;
        g_pSessionManager->kick(session->self, true, reason);
        return {.result = "Kicked '" + data.target + "': " + reason, .good = true}; }}));

	g_pCommandHandler->registerCommand({.requireAdmin = false, .name = "list", .parser = [](const std::string &args) -> CCommandHandler::SParseResult {
        (void)args;
        return {.data = std::any(), .good = true}; }, .exe = [](const std::any &parsed) -> CCommandHandler::SResult {
        (void)parsed;
        std::string result = "ID \t Name \t IP \t Muted\n";
        for (const auto &session : g_pSessionManager->getSessions())
			result += NFormatter::fmt(NONEWLINE, "{} \t {} \t {} \t {}\n", (uintptr_t)session.get(), session->getName(), session->getIp(), (session->isMuted() ? "[MUTED]" : "[UNMUTED]"));


        return {.result = result, .good = true}; }});

	g_pCommandHandler->registerCommand(CCommandHandler::makeCommand<SMuteArgs>({.name = "mute", .parser = [](const std::string &args) -> std::pair<SMuteArgs, bool> {
        size_t pos = args.find(' ');
        std::string target = args;
		std::optional<std::string> reason;

        if (pos != std::string::npos) {
            target = args.substr(0, pos);
			reason = args.substr(pos + 1);
        }

        if (target.empty())
            return {SMuteArgs{}, false};

        return {SMuteArgs{.target=target, .reason=reason}, true}; }, .exe = [](const SMuteArgs &data) -> CCommandHandler::SResult {
        const auto session = g_pSessionManager->getByName(data.target);
        if (!session)
            return {.result = "User '" + data.target + "' not found", .good = false};

		const auto reason = data.reason ? *data.reason : NOREASON;

        session->setMuted(true);
        session->write("You have been muted for: {}", reason);
        return {.result = "Muted '" + data.target + "': " + reason, .good = true}; }})); // TODO: use fmt

	g_pCommandHandler->registerCommand(CCommandHandler::makeCommand<SMuteArgs>({.name = "unmute", .parser = [](const std::string &args) -> std::pair<SMuteArgs, bool> {
        if (args.empty())
            return {SMuteArgs{}, false};

        return {SMuteArgs{.target=args, .reason="Unmuted"}, true}; }, .exe = [](const SMuteArgs &data) -> CCommandHandler::SResult {
        const auto session = g_pSessionManager->getByName(data.target);
        if (!session)
            return {.result = "User '" + data.target + "' not found", .good = false};

        session->setMuted(false);
        session->write("You have been unmuted");
        return {.result = "Unmuted '" + data.target + "'", .good = true}; }}));
}
