#include "TileSet.h"
#include "json.hpp"
using json = nlohmann::json;

const std::map<SubTile::SubPosition, sf::FloatRect> SubTile::subPosRects {
	{ SubTile::full, sf::FloatRect(0, 0, 1, 1) },
	{ SubTile::tlCorner, sf::FloatRect(0, 0, .5, .5) },
	{ SubTile::trCorner, sf::FloatRect(.5, 0, .5, .5) },
	{ SubTile::blCorner, sf::FloatRect(0, .5, .5, .5) },
	{ SubTile::brCorner, sf::FloatRect(.5, .5, .5, .5) },
	{ SubTile::topHalf, sf::FloatRect(0, 0, 1, .5) },
	{ SubTile::botHalf, sf::FloatRect(0, .5, 1, .5) },
	{ SubTile::leftHalf, sf::FloatRect(0, 0, .5, 1) },
	{ SubTile::rightHalf, sf::FloatRect(.5, 0, .5, 1) }
};

std::map<std::string, std::unique_ptr<TileSet>> TileSet::tileSets {};

NLOHMANN_JSON_SERIALIZE_ENUM(Tile::Category, {
	{Tile::Category::terrain_top, "terrain top"},
	{Tile::Category::terrain_wall, "terrain wall"},
	{Tile::Category::terrain_foot, "terrain foot"}
})

NLOHMANN_JSON_SERIALIZE_ENUM(SubTile::Pattern, {
	{SubTile::Pattern::center, "center"},
	{SubTile::Pattern::patch, "patch"},
	{SubTile::Pattern::cross, "cross"},
	{SubTile::Pattern::horizontal, "horizontal"},
	{SubTile::Pattern::vertical, "vertical"},
	{SubTile::Pattern::edges, "edges"}
})

TileSet const& TileSet::get(std::string const& name) {
	auto it = tileSets.find(name);
	if (it == tileSets.end()) {
		// unique_ptr takes control of this new; make_ptr cannot be used here due to TileSet's private constructor
		return *tileSets.emplace(name, new TileSet(name)).first->second;
	}
	else
		return *it->second;
}

void TileSet::unload(std::string const& name) {
	auto it = tileSets.find(name);
	if (it != tileSets.end()) {
		tileSets.erase(it);
	}
	else {
		throw GameError("Tried unloading non-loaded tileset " + name);
	}
}

void TileSet::unload_all() {
	tileSets.clear();
}

TileSet::TileSet(std::string const& name) {
	texture = sf::Texture();
	std::string filename = "resources/" + name + ".png";
	if (!texture.loadFromFile(filename)) {
		throw GameError("No texture file found for tileset " + name + " (expected " + filename + ')');
	}

	filename = "resources/" + name + ".json";
	std::ifstream ifs(filename);
	if (!ifs.is_open()) {
		throw GameError("No json file found for tileset " + name + " (expected " + filename + ')');
	}
	json jFile;
	ifs >> jFile;
	ifs.close();

	//Returns a vector of coordinate variants
	auto getCoords = [this](json const& j) {
		std::vector<sf::Vector2f> v;
		if (j[0].type_name() == std::string("number")) {
			v.emplace_back(tileSize * j[0].get<float>(), tileSize * j[1].get<float>());
			return v;
		}
		for (auto& coords : j) {
			v.emplace_back(tileSize * coords[0].get<float>(), tileSize * coords[1].get<float>());
		}
		return v;
	};

	auto getSubCoords = [this](sf::Vector2f coords, SubTile::SubPosition subPos) {
		float s = (float) tileSize;
		sf::FloatRect rect = SubTile::subPosRects.at(subPos);
		return SubTile::TextureCoords{
			coords + s * sf::Vector2f(rect.left, rect.top),
			coords + s * sf::Vector2f(rect.left + rect.width, rect.top),
			coords + s * sf::Vector2f(rect.left, rect.top + rect.height),
			coords + s * sf::Vector2f(rect.left + rect.width, rect.top + rect.height)
		};
	};

	uint currentTileID = 0;
	uint currentSubTileID = 0;

	auto createSubTiles = [this, &getSubCoords, &currentSubTileID](TileInfo& t,
										 std::vector<sf::Vector2f> const& coords,
										 SubTile::Pattern pattern = SubTile::center,
										 SubTile::SubPosition subPos = SubTile::full) {
		auto& subTiles = t.subTiles[pattern][subPos];
		subTiles.reserve(coords.size());
		for (size_t variant = 0; variant < coords.size(); variant++) {
			SubTile& st = subTiles.emplace_back();
			st.pattern = pattern;
			st.subPosition = subPos;
			st.n_variants = coords.size();
			st.variant = variant;
			st.texCoords = getSubCoords(coords[variant], subPos);
			st.ID = currentSubTileID;
			subTilesByID[currentSubTileID] = &st;
			currentSubTileID++;
		}
	};

	for (auto& [tileName, jTile] : jFile.items()) {
		TileInfo& t = tiles[tileName];
		t.name = tileName;
		t.ID = currentTileID;
		for (auto& [sCategory, compatName] : jTile["compatibility"].items()) {
			Tile::Category category = json(sCategory).get<Tile::Category>();
			t.compatibilities[category] = compatName;
		}

		t.category = jTile["category"].get<Tile::Category>();
		switch (t.category) {
			case Tile::Category::terrain_top: {
				for (std::string sPattern : {"center", "patch", "cross", "horizontal", "vertical"}) {
					SubTile::Pattern ePattern = json(sPattern).get<SubTile::Pattern>();
					auto coords = getCoords(jTile["patterns"][sPattern]["coords"]);
					for (SubTile::SubPosition subPos : { SubTile::full, SubTile::tlCorner, SubTile::trCorner, SubTile::blCorner, SubTile::brCorner }) {
						createSubTiles(t, coords, ePattern, subPos);
					}
				}
				break;
			}
			case Tile::Category::terrain_wall:
			case Tile::Category::terrain_foot: {
				auto coords = getCoords(jTile["patterns"]["center"]["coords"]);
				createSubTiles(t, coords, SubTile::center, SubTile::botHalf);
				createSubTiles(t, coords, SubTile::center, SubTile::blCorner);
				createSubTiles(t, coords, SubTile::center, SubTile::brCorner);

				coords = getCoords(jTile["patterns"]["edges"]["coords"]);
				createSubTiles(t, coords, SubTile::edges, SubTile::botHalf);
				createSubTiles(t, coords, SubTile::edges, SubTile::blCorner);
				createSubTiles(t, coords, SubTile::edges, SubTile::brCorner);
				break;
			}
			default: break;
		}

		currentTileID++;
	}
}

sf::Texture const& TileSet::getTexture() const {
	return texture;
}

Tile::Category TileSet::getCategory(std::string const& name) const {
	auto it = tiles.find(name);
	if (it != tiles.end()) {
		return it->second.category;
	}
	throw GameError("Tried to get category of unknown tile " + name + " from tileset");
}

Tile TileSet::getEmptyTile(std::string const& name) const {
	auto it = tiles.find(name);
	if (it != tiles.end()) {
		Tile t;
		t.info = &it->second;
		return t;
	}
	throw GameError("Tried to load unknown tile " + name + " from tileset");
}

SubTile const* TileSet::getSubTile(std::string const& name,
							  SubTile::Pattern pattern,
							  SubTile::SubPosition subPos,
							  size_t variant) const {
	try {
		return &tiles.at(name).subTiles.at(pattern).at(subPos).at(variant);
	}
	catch (std::out_of_range) {
		throw GameError("Tried to load invalid subtile from " + name
			+ " (pattern " + json(pattern).get<std::string>() + ", subposition " + std::to_string(subPos)
			+ ", variant " + std::to_string(variant) + ')');
	}
}

SubTile const* TileSet::getSubTile(Tile const& tile,
							  SubTile::Pattern pattern,
							  SubTile::SubPosition subPos,
							  size_t variant) const {
	try {
		return &tile.info->subTiles.at(pattern).at(subPos).at(variant);
	}
	catch (std::out_of_range) {
		throw GameError("Tried to load invalid subtile from " + tile.info->name
			+ " (pattern " + json(pattern).get<std::string>() + ", subposition " + std::to_string(subPos)
			+ ", variant " + std::to_string(variant) + ')');
	}
}

SubTile const* TileSet::getSubTile(uint ID) const {
	try {
		return subTilesByID.at(ID);
	}
	catch (std::out_of_range) {
		throw GameError("Tried to load invalid subtile of ID " + std::to_string(ID));
	}
}