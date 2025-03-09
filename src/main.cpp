#include "chatServer.hpp"

// TODO: ARGC, ARGV. the CServer ctor should take a port number
int main() {
  g_pChatServer = std::make_unique<CChatServer>();
  g_pChatServer->start();

  // died
  g_pChatServer->cleanup();
  g_pChatServer.reset();

  return 0;
}
