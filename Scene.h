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

	class Chunk {
	public:
		struct TileCoords {
			int x, y, subz;

			struct RenderOrderComparator {
				inline bool operator()(TileCoords const& before, TileCoords const& after) const;
			};
		};

		void setTile(Tile const& t, TileCoords coords);
		Tile const* getTile(TileCoords coords) const;

		sf::VertexBuffer const& getBuffer(float zOffset) const;

		static const int resolution = 8;

	private:
		std::map<TileCoords, Tile, TileCoords::RenderOrderComparator> tiles;
		mutable sf::VertexBuffer buffer;
		mutable bool shouldUpdateBuffer = false;
	};

	struct ChunkCoords {
		int X, Y, z;

		static inline ChunkCoords fromTileCoords(int x, int y, int z);

		struct RenderOrderComparator {
			using is_transparent = void;
			inline bool operator()(ChunkCoords const& before, ChunkCoords const& after) const;
		};

		inline bool operator==(ChunkCoords const& right) const;
	};

	std::map<ChunkCoords, Chunk, ChunkCoords::RenderOrderComparator> chunks;

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
};
