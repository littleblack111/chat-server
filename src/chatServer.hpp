#include <memory>

class CChatServer {
public:
    CChatServer();
    ~CChatServer();
    void start();
    void cleanup();
private:
  void initManagers();
  void initServer();
};

inline std::unique_ptr<CChatServer> g_pChatServer;

