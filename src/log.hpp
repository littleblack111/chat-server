#pragma once
#include "format.hpp"
#include <format>
#include <print>
#include <string>
#include <unistd.h>

inline void log(FILE *_stream, eFormatType type, std::string str) {
#ifndef DEBUG
	if (type == TRACE)
		return;
#endif
	std::println(_stream, "{}", NFormatter::fmt(type, str));
}

template <typename... Args>
void log(FILE *_stream, eFormatType type, std::format_string<Args...> fmt, Args &&...args) {
#ifndef DEBUG
	if (type == TRACE)
		return;
#endif
	std::println(_stream, "{}", NFormatter::fmt(type, fmt, std::forward<Args>(args)...));
}

inline void log(eFormatType type, std::string str) {
	log(type == ERR ? stderr : stdout, type, str);
}

template <typename... Args>
void log(eFormatType type, std::format_string<Args...> fmt, Args &&...args) {
	log(type == ERR ? stderr : stdout, type, fmt, std::forward<Args>(args)...);
}
