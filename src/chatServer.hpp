#include <memory>
#include <thread>

class CChatServer {
  public:
	CChatServer(uint16_t port);
	~CChatServer();
	void start();
	void cleanup();

  private:
	uint16_t m_port;

	void initManagers();
	void initServer();

	std::jthread m_sessionManagerThread;
};

inline std::unique_ptr<CChatServer> g_pChatServer;
