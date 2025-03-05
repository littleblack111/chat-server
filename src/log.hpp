#pragma once
#include "format.hpp"
#include <chrono>
#include <format>
#include <print>
#include <string>

enum eLogLevel : int8_t {
	INFO = 0,
	WARN,
	ERROR,
	FATAL
};

inline bool useColors = true;

void log(eLogLevel level, std::string str);

template <typename... Args>
void log(eLogLevel level, std::format_string<Args...> fmt, Args &&...args) {
	const auto	now		  = std::chrono::system_clock::now();
	const auto	hms		  = std::chrono::hh_mm_ss{now - std::chrono::floor<std::chrono::days>(now)};
	std::string timeStamp = std::format("[{}] ", hms);

	std::string msg = CFormatter::fmt(eFormatType::RAW, fmt, std::forward<Args>(args)...);

	log(level, timeStamp + msg);
}
