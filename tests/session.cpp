#include "session.hpp"

using RecvData = ::CSession::SRecvData;

bool NTests::CSession::SRecvData::sanitize() {
	RecvData		  data;
	const std::string beforeSanitize = " a\n";
	const std::string afterSanitize	 = "a";

	data.data = beforeSanitize;
	return data.data == afterSanitize;
}

bool NTests::CSession::SRecvData::isEmpty() {
	RecvData		  data1, data2, data3, data4;
	const std::string emptyWithSpace   = "    ";
	const std::string emptywithnewline = "\n";
	const std::string spacewithnewline = "   \n   ";
	const std::string empty			   = "";

	data1.data = emptyWithSpace;
	data2.data = emptywithnewline;
	data3.data = spacewithnewline;
	data4.data = empty;

	return data1.isEmpty() && data2.isEmpty() && data3.isEmpty() && data4.isEmpty();
}
