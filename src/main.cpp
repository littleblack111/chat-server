#include "chatServer.hpp"
#include "log.hpp"
#include <csignal>
#include <unistd.h>

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

// TODO: ARGC, ARGV. the CServer ctor should take a port number
int main() {
	registerSignals();

	g_pChatServer = std::make_unique<CChatServer>();
	g_pChatServer->start();

	// died
	g_pChatServer->cleanup();
	g_pChatServer.reset();

	return 0;
}
