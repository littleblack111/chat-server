#include <memory>
#include <string>
#include <vector>

class CChatManager {
public:
  CChatManager() = default;
  ~CChatManager();
  struct SMessage {
    std::string msg;
    std::string username;
  };
  void newMessage(const SMessage &msg);
  void broadcastMessage(const SMessage &msg);
private:
  void broadcastMessage(const std::string &msg, const std::string &username);
  std::vector<SMessage> m_vMessages;
};
inline std::unique_ptr<CChatManager> g_pChatManager;
