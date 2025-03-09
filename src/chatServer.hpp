#include <memory>
#include <thread>

class CChatServer {
public:
    CChatServer();
    ~CChatServer();
    void start();
    void cleanup();
private:
  void initManagers();
  void initServer();
  std::jthread m_sessionThread;
};

inline std::unique_ptr<CChatServer> g_pChatServer;

