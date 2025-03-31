#pragma once
#include "format.hpp"
#include <string>
#include <print>
#include "IOManager.hpp"

inline void log(FILE *_stream, eFormatType type, std::string str) {
#ifndef DEBUG
	if (type == TRACE)
		return;
#endif
  if (g_pIOManager)
    g_pIOManager->addLog({.type = type, .log = str});
  else
    std::println(_stream, "{}", NFormatter::fmt(type, str));
}

template <typename... Args>
void log(FILE *_stream, eFormatType type, std::format_string<Args...> fmt, Args &&...args) {
#ifndef DEBUG
	if (type == TRACE)
		return;
#endif
  if (g_pIOManager)
    g_pIOManager->addLog({.type = type, .log = NFormatter::fmt(NONEWLINE, fmt, std::forward<Args>(args)...)});
  else
    std::println(_stream, "{}", NFormatter::fmt(type, fmt, std::forward<Args>(args)...));
}

inline void log(eFormatType type, std::string str) {
	log(type == ERR ? stderr : stdout, type, str);
}

template <typename... Args>
void log(eFormatType type, std::format_string<Args...> fmt, Args &&...args) {
	log(type == ERR ? stderr : stdout, type, fmt, std::forward<Args>(args)...);
}
