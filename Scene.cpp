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
	if (before.subz < after.subz)
		return true;
	return false;
}

void Scene::Chunk::setTile(Tile const& t, Scene::Chunk::TileCoords coords) {
	tiles[coords] = t;
	shouldUpdateBuffer = true;
}

Tile const* Scene::Chunk::getTile(Scene::Chunk::TileCoords coords) const {
	auto it = tiles.find(coords);
	return it == tiles.end() ? nullptr : &it->second;
}

sf::VertexBuffer const& Scene::Chunk::getBuffer(float zOffset) const {
	if (shouldUpdateBuffer) {
		size_t nQuads = 0;
		for (auto it : tiles) {
			nQuads += it.second.subTiles.size();
		}

		sf::Vertex* array = new sf::Vertex[nQuads * 6];
		size_t bufferIndex = 0;

		for (auto& [coords, tile] : tiles) {
			for (auto subTile : tile.subTiles) {
				sf::FloatRect rect = SubTile::subPosRects.at(subTile->subPosition);

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
			}
		}

		buffer.create(nQuads * 6);
		buffer.setUsage(sf::VertexBuffer::Usage::Static);
		buffer.setPrimitiveType(sf::PrimitiveType::Triangles);
		buffer.update(array);
		delete[] array;
		shouldUpdateBuffer = false;
	}

	return buffer;
}

inline bool Scene::ChunkCoords::RenderOrderComparator::operator()(ChunkCoords const& before, ChunkCoords const& after) const {
	if (before.Y < after.Y)
		return true;
	if (before.Y > after.Y)
		return false;
	if (before.X < after.X)
		return true;
	if (before.X > after.X)
		return false;
	if (before.z < after.z)
		return true;
	return false;
}

void Scene::setTile(Tile const& t, int x, int y, int z, int subz) {
	chunks[ChunkCoords::fromTileCoords(x, y, z)].setTile(t, { x, y, subz });
}

Tile const* Scene::getTile(int x, int y, int z, int subz) const {
	auto it = chunks.find(ChunkCoords::fromTileCoords(x, y, z));
	return it == chunks.end() ? nullptr : it->second.getTile({ x, y, subz });
}

TileSet const& Scene::getTileSet() const {
	return tileset;
}

int Scene::getLowestTileHeight(int x, int y, int subz, int start_z) const {
	ChunkCoords cCoords = ChunkCoords::fromTileCoords(x, y, start_z);
	for (auto it = chunks.lower_bound(cCoords);
		 it != chunks.end() && it->first.X == cCoords.X && it->first.Y == cCoords.Y;
		 it++)
	{
		Tile const* t = it->second.getTile({ x, y, subz });
		if (t != nullptr)
			return it->first.z;
	}
	return std::numeric_limits<int>::max();
}

int Scene::getHighestTileHeight(int x, int y, int subz, int start_z) const {
	ChunkCoords cCoords = ChunkCoords::fromTileCoords(x, y, start_z);

	for (auto it = std::make_reverse_iterator(chunks.upper_bound(cCoords));
		 it != chunks.rend() && it->first.X == cCoords.X && it->first.Y == cCoords.Y;
		 it++)
	{
		Tile const* t = it->second.getTile({ x, y, subz });
		if (t != nullptr)
			return it->first.z;
	}
	return std::numeric_limits<int>::min();
}

void Scene::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	states.transform *= getTransform();
	states.texture = &tileset.getTexture();

	for (auto& [coords, chunk] : chunks) {
		target.draw(chunk.getBuffer(-(float)coords.z / 2), states);
	}
}

inline Scene::ChunkCoords Scene::ChunkCoords::fromTileCoords(int x, int y, int z) {
	return ChunkCoords{
		x >= 0 ? x / Chunk::resolution : -(-x / Chunk::resolution + 1),
		y >= 0 ? y / Chunk::resolution : -(-y / Chunk::resolution + 1),
		z
	};
}

inline bool Scene::ChunkCoords::operator==(ChunkCoords const& right) const {
	return X == right.X && Y == right.Y && z == right.z;
}
