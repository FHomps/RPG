#include "Error.h"

GameError::GameError() {}

GameError::GameError(std::string const& msg) : message(msg) {}

const char* GameError::what() const noexcept {
	return message.c_str();
}
