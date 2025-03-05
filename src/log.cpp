#include "log.hpp"

void log(eLogLevel level, std::string str) {
	// Use CFormatter for the prefix part
	std::string prefixed;
	switch (level) {
	case INFO:
		prefixed = CFormatter::fmt<const std::string &>(eFormatType::STANDARD, "{}", str);
		str		 = useColors ? "\033[1;32m" + prefixed + "\033[0m" : prefixed; // green
		break;
	case WARN:
		prefixed = CFormatter::fmt<const std::string &>(eFormatType::SYSTEM, "{}", str);
		str		 = useColors ? "\033[1;33m" + prefixed + "\033[0m" : prefixed; // yellow
		break;
	case ERROR:
		prefixed = CFormatter::fmt<const std::string &>(eFormatType::ERROR, "{}", str);
		str		 = useColors ? "\033[1;31m" + prefixed + "\033[0m" : prefixed; // red
		break;
	case FATAL:
		prefixed = CFormatter::fmt<const std::string &>(eFormatType::ERROR, "{}", str);
		str		 = useColors ? "\033[1;35m" + prefixed + "\033[0m" : prefixed; // magenta
		break;
	}

	std::println("{}", str); // Changed to print since we're managing the newline ourselves
}
