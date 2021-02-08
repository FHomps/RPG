#include "PCH.h"
#include "Scene.h"

inline bool Scene::Chunk::TileCoords::RenderOrderComparator::operator()(TileCoords const& before, TileCoords const& after) const {
	if (before.y < after.y)
		return true;
	if (before.y > after.y)
		return false;
	if (before.x < after.x)
		return true;
	if (before.x > after.x)
		return false;
	if (before.z < after.z)
		return true;
	if (before.z > after.z)
		return false;
	if (before.subz < after.subz)
		return true;
	return false;
}

		/*
		sf::Vector2f offset = sf::Vector2f((float)coords.x, (float)coords.y + zOffset);
		array[bufferIndex].position = offset + sf::Vector2f(rect.left, rect.top);
		array[bufferIndex + 1].position = offset + sf::Vector2f(rect.left + rect.width, rect.top);
		array[bufferIndex + 2].position = offset + sf::Vector2f(rect.left, rect.top + rect.height);
		array[bufferIndex + 3].position = offset + sf::Vector2f(rect.left, rect.top + rect.height);
		array[bufferIndex + 4].position = offset + sf::Vector2f(rect.left + rect.width, rect.top);
		array[bufferIndex + 5].position = offset + sf::Vector2f(rect.left + rect.width, rect.top + rect.height);

		array[bufferIndex + 0].texCoords = subTile->texCoords.topLeft;
		array[bufferIndex + 1].texCoords = subTile->texCoords.topRight;
		array[bufferIndex + 2].texCoords = subTile->texCoords.bottomLeft;
		array[bufferIndex + 3].texCoords = subTile->texCoords.bottomLeft;
		array[bufferIndex + 4].texCoords = subTile->texCoords.topRight;
		array[bufferIndex + 5].texCoords = subTile->texCoords.bottomRight;

		bufferIndex += 6;
		*/

inline bool Scene::ChunkCoords::Comparator::operator()(ChunkCoords const& before, ChunkCoords const& after) const {
	if (before.Y < after.Y)
		return true;
	if (before.Y > after.Y)
		return false;
	if (before.X < after.X)
		return true;
	return false;
}

void Scene::setTile(Tile const& t, int x, int y, int z, int subz) {
	chunks[ChunkCoords::fromTileCoords(x, y)].tiles[{ x, y, z, subz }] = t;

	//TODO sprites should be loaded into the currentSprites map at chunk loading instead
	SpriteCoords sc{ (float)x, (float)y, (float)z, subz };
	currentSprites.erase(sc);
	for (auto& subtile : t.subTiles) {
		currentSprites.insert({ sc, {
			subtile->textureRect,
			SubTile::subPosRects.at(subtile->subPosition)
		} });
	}
}

Tile const* Scene::getTile(int x, int y, int z, int subz) const {
	auto it_c = chunks.find(ChunkCoords::fromTileCoords(x, y));
	if (it_c != chunks.end()) {
		auto it_t = it_c->second.tiles.find({ x, y, z, subz });
		if (it_t != it_c->second.tiles.end())
			return &it_t->second;
	}
	return nullptr;
}

TileSet const& Scene::getTileSet() const {
	return tileset;
}

int Scene::getLowestTileHeight(int x, int y, int subz, int min_height) const {
	auto it_c = chunks.find(ChunkCoords::fromTileCoords(x, y));
	if (it_c != chunks.end()) {
		auto const& tiles = it_c->second.tiles;
		for (auto it_t = tiles.lower_bound({ x, y, min_height, subz });
			it_t != tiles.end() && it_t->first.x == x && it_t->first.y == y;
			it_t++)
		{
			if (it_t->first.subz == subz)
				return it_t->first.z;
		}
	}
	return std::numeric_limits<int>::max();
}

int Scene::getHighestTileHeight(int x, int y, int subz, int max_height) const {
	auto it_c = chunks.find(ChunkCoords::fromTileCoords(x, y));
	if (it_c != chunks.end()) {
		auto const& tiles = it_c->second.tiles;
		for (auto it_t = std::make_reverse_iterator(tiles.upper_bound({ x, y, max_height, subz }));
			it_t != tiles.rend() && it_t->first.x == x && it_t->first.y == y;
			it_t++)
		{
			if (it_t->first.subz == subz)
				return it_t->first.z;
		}
	}
	return std::numeric_limits<int>::min();
}

void Scene::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	states.transform *= getTransform();
	states.texture = &tileset.getTexture();
	sf::VertexArray va(sf::PrimitiveType::Triangles, currentSprites.size() * 6);
	int i = 0;
	for (auto const& [coords, sprite] : currentSprites) {
		sf::Vector2f posOffset{ coords.x, coords.y - coords.z / 2 };
		va[i].position = posOffset + sf::Vector2f{ sprite.shapeRect.left, sprite.shapeRect.top };
		va[i + 1].position = posOffset + sf::Vector2f{ sprite.shapeRect.left + sprite.shapeRect.width, sprite.shapeRect.top };
		va[i + 2].position = posOffset + sf::Vector2f{ sprite.shapeRect.left, sprite.shapeRect.top + sprite.shapeRect.height };
		va[i + 3].position = va[i + 2].position;
		va[i + 4].position = va[i + 1].position;
		va[i + 5].position = posOffset + sf::Vector2f{ sprite.shapeRect.left + sprite.shapeRect.width, sprite.shapeRect.top + sprite.shapeRect.height };

		va[i].texCoords = { sprite.textureRect.left, sprite.textureRect.top };
		va[i + 1].texCoords = { sprite.textureRect.left + sprite.textureRect.width, sprite.textureRect.top };
		va[i + 2].texCoords = { sprite.textureRect.left, sprite.textureRect.top + sprite.textureRect.height };
		va[i + 3].texCoords = va[i + 2].texCoords;
		va[i + 4].texCoords = va[i + 1].texCoords;
		va[i + 5].texCoords = { sprite.textureRect.left + sprite.textureRect.width, sprite.textureRect.top + sprite.textureRect.height };
		
		i += 6;
	}
	target.draw(va, states);
}

inline Scene::ChunkCoords Scene::ChunkCoords::fromTileCoords(int x, int y) {
	return ChunkCoords{
		x >= 0 ? x / Chunk::resolution : -(-x / Chunk::resolution + 1),
		y >= 0 ? y / Chunk::resolution : -(-y / Chunk::resolution + 1)
	};
}

inline bool Scene::SpriteCoords::RenderOrderComparator::operator()(SpriteCoords const& before, SpriteCoords const& after) const {
	if (before.y < after.y)
		return true;
	if (before.y > after.y)
		return false;
	if (before.x < after.x)
		return true;
	if (before.x > after.x)
		return false;
	if (before.z < after.z)
		return true;
	if (before.z > after.z)
		return false;
	if (before.subz < after.subz)
		return true;
	return false;
}
