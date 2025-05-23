#include "../src/session.hpp"

namespace NTests {
  struct CSession {
    CSession();
    ~CSession();
    namespace SRecvData {
      bool sanitize();
      bool isEmpty();
    }
  }
}
