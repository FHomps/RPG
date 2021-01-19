#pragma once

template<class T>
std::string vec2ToString(sf::Vector2<T> v) {
	return '(' + std::to_string(v.x) + ", " + std::to_string(v.y) + ')';
}

template<class T>
std::string vec3ToString(sf::Vector3<T> v) {
	return '(' + std::to_string(v.x) + ", " + std::to_string(v.y) + ", " + std::to_string(v.z) + ')';
}

sf::Color randomColor();

sf::Vector2u indexToCoords(uint i, sf::Vector2u size);

uint coordsToIndex(sf::Vector2u coords, sf::Vector2u size);