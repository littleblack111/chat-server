#pragma once
#include <format>
#include <string>

enum eFormatType : int8_t {
	NONE,
	TRACE,
	LOG,
	WARN,
	ERR,
	SYS,
  NONEWLINE
};

namespace NFormatter {
inline std::string fmt(eFormatType type, std::string str) {
	std::string prefix;
	switch (type) {
	case LOG:
		prefix = "[LOG] ";
		break;
	case WARN:
		prefix = "[WARN] ";
		break;
	case ERR:
		prefix = "[ERROR] ";
		break;
	case SYS:
		prefix = "[SYSTEM] ";
		break;
	case TRACE:
		prefix = "[DEBUG] ";
		break;
	default:
		prefix = "";
		break;
	}

	if (str.back() != '\n' && type != NONEWLINE)
		str += '\n';

	return prefix + str;
}

template <typename... Args>
std::string fmt(eFormatType type, std::format_string<Args...> fmt, Args &&...args) {
	return NFormatter::fmt(type, std::format(fmt, std::forward<Args>(args)...));
}
} // namespace NFormatter
