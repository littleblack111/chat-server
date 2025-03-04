#include "sessionManager.hpp"
#include "server.hpp"

int main() {
  g_pServer = std::make_unique<CServer>(8080);
  g_pSessionManager = std::make_unique<CSessionManager>();
  g_pSessionManager->enterLoop();
	return 0;
}
