#include "../src/session.hpp"

namespace NTests {
struct CSession { // NOLINT
	CSession();
	~CSession();
	struct SRecvData {
		bool sanitize();
		bool isEmpty();
	};
};
} // namespace NTests
