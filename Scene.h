#pragma once
#include "TileSet.h"

class Scene : public sf::Drawable, public sf::Transformable {
public:
	Scene(TileSet const& tileset) : tileset(tileset) {}

	void setTile(Tile const& t, int x, int y, int z, int subz = 0);
	Tile const* getTile(int x, int y, int z, int subz = 0) const;

	TileSet const& getTileSet() const;

	int getLowestTileHeight(int x, int y, int subz = 0, int min_height = std::numeric_limits<int>::min()) const;
	int getHighestTileHeight(int x, int y, int subz = 0, int max_height = std::numeric_limits<int>::max()) const;

private:
	TileSet const& tileset;

	struct Chunk {
		struct TileCoords {
			int x, y, z, subz;

			struct RenderOrderComparator {
				inline bool operator()(TileCoords const& before, TileCoords const& after) const;
			};
		};

		std::map<TileCoords, Tile, TileCoords::RenderOrderComparator> tiles;

		static const int resolution = 8;
	};

	struct ChunkCoords {
		int X, Y;

		static inline ChunkCoords fromTileCoords(int x, int y);
		static inline ChunkCoords fromTileCoords(Chunk::TileCoords const& coords);

		struct Comparator {
			inline bool operator()(ChunkCoords const& before, ChunkCoords const& after) const;
		};
	};

	struct SpriteCoords {
		float x, y, z;
		int subz;

		struct RenderOrderComparator {
			inline bool operator()(SpriteCoords const& before, SpriteCoords const& after) const;
		};
	};

	struct Sprite {
		sf::FloatRect textureRect;
		sf::FloatRect shapeRect;
	};

	std::map<ChunkCoords, Chunk, ChunkCoords::Comparator> chunks;

	std::multimap<SpriteCoords, Sprite, SpriteCoords::RenderOrderComparator> currentSprites;

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
};
