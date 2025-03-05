#pragma once
#include <format>
#include <string>

enum class eFormatType {
	RAW,	  // No formatting at all
	STANDARD, // Standard message format
	SYSTEM,	  // System message format
	ERROR,	  // Error message format
	CHAT,	  // Chat message format (username: message)
	INFO,	  // Info level log format
	WARN,	  // Warning level log format
	FATAL	  // Fatal level log format
};

class CFormatter {
  public:
	template <typename... Args>
	static std::string fmt(eFormatType type, std::format_string<Args...> fmt, Args &&...args, bool addNewline = true) {
		std::string formatted = std::vformat(fmt.get(), std::make_format_args(args...));
		if (addNewline && !formatted.empty() && formatted.back() != '\n')
			formatted += '\n';

		switch (type) {
		case eFormatType::RAW:
			return formatted;
		case eFormatType::STANDARD:
			return "[MSG] " + formatted;
		case eFormatType::SYSTEM:
			return "[SYS] " + formatted;
		case eFormatType::ERROR:
			return "[ERR] " + formatted;
		case eFormatType::CHAT:
			return formatted; // handled by chatFmt
		case eFormatType::INFO:
			return "[INFO] " + formatted;
		case eFormatType::WARN:
			return "[WARN] " + formatted;
		case eFormatType::FATAL:
			return "[FATAL] " + formatted;
		default:
			return formatted;
		}
	}

	template <typename... Args>
	static std::string chatFmt(const std::string &username, std::format_string<Args...> fmt, Args &&...args) {
		std::string formatted = std::vformat(fmt.get(), std::make_format_args(args...));
		return std::format("{}: {}", username, formatted);
	}
};

template <typename... Args>
inline std::string fmtRaw(std::format_string<Args...> fmt, Args &&...args, bool addNewline = true) {
	return CFormatter::fmt(eFormatType::RAW, fmt, std::forward<Args>(args)..., addNewline);
}

template <typename... Args>
inline std::string fmtStd(std::format_string<Args...> fmt, Args &&...args, bool addNewline = true) {
	return CFormatter::fmt(eFormatType::STANDARD, fmt, std::forward<Args>(args)..., addNewline);
}

template <typename... Args>
inline std::string fmtSys(std::format_string<Args...> fmt, Args &&...args, bool addNewline = true) {
	return CFormatter::fmt(eFormatType::SYSTEM, fmt, std::forward<Args>(args)..., addNewline);
}

template <typename... Args>
inline std::string fmtErr(std::format_string<Args...> fmt, Args &&...args, bool addNewline = true) {
	return CFormatter::fmt(eFormatType::ERROR, fmt, std::forward<Args>(args)..., addNewline);
}

template <typename... Args>
inline std::string fmtChat(const std::string &username, std::format_string<Args...> fmt, Args &&...args) {
	return CFormatter::chatFmt(username, fmt, std::forward<Args>(args)...);
}
