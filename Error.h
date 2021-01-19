#pragma once

class GameError : public std::exception {
public:
	GameError();
	explicit GameError(std::string const& msg);
	const char* what() const noexcept override;

private:
	std::string message;
};