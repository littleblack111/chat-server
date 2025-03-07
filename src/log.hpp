#pragma once
#include "format.hpp"
#include <format>
#include <print>
#include <string>

inline void log(FILE *_stream, eFormatType type, std::string str) {
	std::println(_stream, "{}", NFormatter::fmt(type, str));
}

inline void log(eFormatType type, std::string str) {
	log(stdout, type, str);
}

template <typename... Args>
void log(FILE *_stream, eFormatType type, std::format_string<Args...> fmt, Args &&...args) {
	log(_stream, type, std::format(fmt, std::forward<Args>(args)...));
}

template <typename... Args>
void log(eFormatType type, std::format_string<Args...> fmt, Args &&...args) {
	log(stdout, type, std::format(fmt, std::forward<Args>(args)...));
}
