#include <memory>

class CInputManager {
  public:
	CInputManager();
	~CInputManager();

	void run();
	void inputLoop();
};
inline std::unique_ptr<CInputManager> g_pInputManager;
