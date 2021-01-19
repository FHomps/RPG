#include "Tools.h"

sf::Color randomColor() {
	return sf::Color(rand() % UINT8_MAX, rand() % UINT8_MAX, rand() % UINT8_MAX);
}

sf::Vector2u indexToCoords(uint i, sf::Vector2u size) {
	sf::Vector2u coords;
	coords.x = i % size.x;
	coords.y = i / size.x;
	if (coords.x >= size.x || coords.y >= size.y) {
		throw GameError("Tried to convert out of range index to coordinates: " + vec2ToString(coords) + " over size " + vec2ToString(size));
	}
	return coords;
}

uint coordsToIndex(sf::Vector2u coords, sf::Vector2u size) {
	if (coords.x >= size.x || coords.y >= size.y) {
		throw GameError("Tried to convert out of range coordinates to index: " + vec2ToString(coords) + " over size " + vec2ToString(size));
	}
	return coords.x + size.x * coords.y;
}