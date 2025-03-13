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
		signal(SIGALRM, [](int) { std::quick_exit(1); });
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

// TODO: ARGC, ARGV. the CServer ctor should take a port number
int main(int argc, char *argv[]) {
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
      if (std::stoi(arg) < 0) {
        log(ERR, "Port number must be greater than 0");

        return EXIT_FAILURE;
      } else if (std::stoi(arg) > 65535) {
        log(ERR, "Port number must be less than 65536");

        return EXIT_FAILURE;
      }

      port = std::stoi(arg);
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
}
