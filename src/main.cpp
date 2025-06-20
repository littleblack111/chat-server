#include "chatServer.hpp"
#include "log.hpp"
#include <algorithm>
#include <csignal>
#include <unistd.h>
#include <vector>

static void handleSignal(int sig) {
	signal(sig, SIG_DFL);

	log(SYS, "Caught signal: {}", sig);

	switch (sig) {
	default:
		// quick_exit(less cleanup) if the cleanup took too long
		signal(SIGALRM, [](int) { std::_Exit(1); });
		alarm(5);

		std::exit(0);
		break;
	}
}

static void registerSignals() {
	signal(SIGINT, handleSignal);
	signal(SIGTERM, handleSignal);

	// client may not finish reading hence the server will receive SIGPIPE, we don't want that to just crash the server
	signal(SIGPIPE, SIG_IGN);
}

static void help() {
	log(ERR, "usage: chat-server <port>");
}

int main(int argc, char *argv[]) try {
	std::vector<std::string> args{argv + 1, argv + argc};
	if (args.empty()) {
		help();

		return EXIT_FAILURE;
	}
	uint16_t port = 0;
	for (const auto &arg : args) {
		if (arg == "-h" || arg == "--help") {
			help();
			return EXIT_SUCCESS;
		} else if (std::ranges::all_of(arg, ::isdigit)) {
			port = std::stoi(arg);
			if (port < 0 || port > 65535) {
				log(ERR, "Port number must be between 0 and 65535");

				return EXIT_FAILURE;
			}

			log(LOG, "Port number: {}", port);
		}
	}

	if (port == 0) {
		log(ERR, "Port number is required");

		help();
		return EXIT_FAILURE;
	}

	registerSignals();

	g_pChatServer = std::make_unique<CChatServer>(port);
	g_pChatServer->start();

	// died
	g_pChatServer->cleanup();
	g_pChatServer.reset();

	return EXIT_SUCCESS;
} catch (const std::exception &e) {
	log(ERR, "Fatal error: {}", e.what());
	return EXIT_FAILURE;
} catch (...) {
	log(ERR, "Unknown fatal error occurred");
	return EXIT_FAILURE;
}
