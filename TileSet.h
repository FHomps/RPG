#pragma once
#include "json.hpp"

struct SubTile {
	uint ID; //Unique for every subtile; used for saving / loading scenes

	//Pattern used in the texture file
	//This can have different meanings depending on the tile category (e.g. "center" for paths and walls)
	enum Pattern {
		center,
		patch,
		cross,
		horizontal,
		vertical,
		edges
	} pattern = Pattern::center;

	size_t variant;			//Variant of the pattern used (for texture variety)		
	size_t n_variants;		//Number of available variants of the pattern

	//Which part of the pattern is used in this subtile
	enum SubPosition {
		full,
		tlCorner,
		trCorner,
		blCorner,
		brCorner,
		topHalf,
		botHalf,
		leftHalf,
		rightHalf
	} subPosition = SubPosition::full;

	static const std::map<SubTile::SubPosition, sf::FloatRect> subPosRects;
	
	sf::FloatRect textureRect;
};

struct TileInfo;

struct Tile {
	//Category the tile belongs to; defines its physics.
	enum Category : char {
		terrain_top,
		terrain_wall,
		terrain_foot
	};

	TileInfo const* info = nullptr;
	std::vector<SubTile const*> subTiles;
};

struct TileInfo { //Stores information about a tile such as its possible subtiles
	std::string name;
	uint ID; //Unique for every subtile; used for saving / loading scenes

	Tile::Category category;

	std::map<Tile::Category, std::string> compatibilities;

	//Search is done by pattern, then subposition, then variant
	std::map<SubTile::Pattern, std::map<SubTile::SubPosition, std::vector<SubTile>>> subTiles;
};

class TileSet {
public:
	static TileSet const& get(std::string const& name);
	static void unload(std::string const& name);
	static void unload_all();

	sf::Texture const& getTexture() const;

	Tile::Category getCategory(std::string const& name) const;

	Tile getEmptyTile(std::string const& name) const;

	SubTile const* getSubTile(std::string const& name,
							  SubTile::Pattern pattern = SubTile::Pattern::center,
							  SubTile::SubPosition subPos = SubTile::SubPosition::full,
							  size_t variant = 0) const;

	SubTile const* getSubTile(Tile const& tile,
							  SubTile::Pattern pattern = SubTile::Pattern::center,
							  SubTile::SubPosition subPos = SubTile::SubPosition::full,
							  size_t variant = 0) const;

	SubTile const* getSubTile(uint ID) const;

	const uint tileSize = 24;

private:
	TileSet(std::string const& name);

	sf::Texture texture;

	std::map<std::string, TileInfo> tiles;
	std::map<uint, SubTile*> subTilesByID;

	static std::map<std::string, std::unique_ptr<TileSet>> tileSets;
};