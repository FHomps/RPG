#include "SceneEditor.h"
#include "Scene.h"

const std::map<Tile::Category, int> preferredSubZ = {
	{Tile::terrain_top, 0},
	{Tile::terrain_wall, 1},
	{Tile::terrain_foot, 1}
};

void SceneEditionTool::setScene(Scene& scene) {
	this->scene = &scene;
}

void SceneEditionTool::setFilledTile(std::string const& name, int x, int y, int z, bool relayUpdate) const {
	TileSet const& set = scene->getTileSet();
	Tile tile = set.getEmptyTile(name);
	TileInfo const& info = *tile.info;
	int subz = preferredSubZ.at(info.category);

	auto neighbours = [&](std::string const& name, int xOffset, int yOffset, int zOffset) {
		return checkName(x + xOffset, y + yOffset, z + zOffset, name) != nullptr;
	};

	auto updatePotentialNeighbour = [&](std::string const& name, int xOffset, int yOffset, int zOffset, bool relay = true) {
		if (neighbours(name, xOffset, yOffset, zOffset))
			setFilledTile(name, x + xOffset, y + yOffset, z + zOffset, relay);
	};

	const std::vector<std::pair<int, int>> directNeighbours = {{-1, 0}, {0, -1}, {0, 1}, {1, 0}};
	const std::vector<std::pair<int, int>> allNeighbours = {{-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}, {1, 1}};

	switch (info.category) {
	case Tile::terrain_foot: {
		std::string const& top = info.compatibilities.at(Tile::terrain_top);
		std::string const& wall = info.compatibilities.at(Tile::terrain_wall);
		std::string const& foot = info.name;

		bool connectLeft = neighbours(foot, -1, 0, 0) || neighbours(wall, -1, 0, 0);
		bool connectRight = neighbours(foot, 1, 0, 0) || neighbours(wall, 1, 0, 1);

		auto uncoveredTop = [&](int xOffset, int yOffset) {
			return neighbours(top, xOffset, yOffset, 0)
			   && !neighbours(wall, xOffset, yOffset, 0)
			   && !neighbours(foot, xOffset, yOffset, 0);
		};

		bool footLeft = uncoveredTop(0, 1) || (uncoveredTop(-1, 1) && neighbours(top, -1, 0, 0) && neighbours(foot, -1, 0, 0) && neighbours(top, 0, 1, 0));
		bool footRight = uncoveredTop(0, 1) || (uncoveredTop(1, 1) && neighbours(top, 1, 0, 0) && neighbours(foot, 1, 0, 0) && neighbours(top, 0, 1, 0));

		size_t wallVariant = std::abs(z) % 2;

		if ((connectLeft == connectRight) && (footLeft == footRight)) {
			tile.subTiles.push_back(set.getSubTile(footLeft ? foot : wall, connectLeft ? SubTile::center : SubTile::edges, SubTile::botHalf, footLeft ? 0 : wallVariant));
		}
		else {
			tile.subTiles.push_back(set.getSubTile(footLeft ? foot : wall, connectLeft ? SubTile::center : SubTile::edges, SubTile::blCorner, footLeft ? 0 : wallVariant));
			tile.subTiles.push_back(set.getSubTile(footRight ? foot : wall, connectRight ? SubTile::center : SubTile::edges, SubTile::brCorner, footRight ? 0 : wallVariant));
		}
		
		scene->setTile(tile, x, y, z, subz);

		if (relayUpdate) {
			for (auto [xOffset, yOffset]: directNeighbours) {
				updatePotentialNeighbour(foot, 0, xOffset, yOffset, false);
				updatePotentialNeighbour(wall, 0, xOffset, yOffset, false);
			}
		}
		break;
	}
	case Tile::terrain_wall: {
		std::string const& top = info.compatibilities.at(Tile::terrain_top);
		std::string const& wall = info.name;
		std::string const& foot = info.compatibilities.at(Tile::terrain_foot);

		if (neighbours(top, 0, 0, 0)) {
			setFilledTile(foot, x, y, z, true);
			break;
		}

		auto uncoveredTop = [&](int xOffset, int yOffset) {
			return neighbours(top, xOffset, yOffset, 0)
				&& !neighbours(wall, xOffset, yOffset, 0)
				&& !neighbours(foot, xOffset, yOffset, 0);
		};

		bool shouldBuildWallInstead = false;
		for (auto [xOffset, yOffset] : allNeighbours) {
			if (uncoveredTop(xOffset, yOffset)) {
				shouldBuildWallInstead = true;
				break;
			}
		}
		if (shouldBuildWallInstead) {
			setFilledTile(foot, x, y, z, true);
			setFilledTile(top, x, y, z, true);
			break;
		}

		bool connectLeft = neighbours(wall, -1, 0, 0) || neighbours(foot, -1, 0, 0);
		bool connectRight = neighbours(wall, 1, 0, 0) || neighbours(foot, 1, 0, 0);

		size_t wallVariant = std::abs(z) % 2;

		if (connectLeft == connectRight) {
			tile.subTiles.push_back(set.getSubTile(wall, connectLeft ? SubTile::center : SubTile::edges, SubTile::botHalf, wallVariant));
		}
		else {
			tile.subTiles.push_back(set.getSubTile(wall, connectLeft ? SubTile::center : SubTile::edges, SubTile::blCorner, wallVariant));
			tile.subTiles.push_back(set.getSubTile(wall, connectRight ? SubTile::center : SubTile::edges, SubTile::brCorner, wallVariant));
		}

		scene->setTile(tile, x, y, z, subz);

		if (relayUpdate) {
			for (auto [xOffset, yOffset]: directNeighbours) {
				updatePotentialNeighbour(foot, xOffset, yOffset, 0, false);
				updatePotentialNeighbour(wall, xOffset, yOffset, 0, false);
			}
		}
		break;
	}
	case Tile::terrain_top: {
		std::string const& top = info.name;
		std::string const& wall = info.compatibilities.at(Tile::terrain_wall);
		std::string const& foot = info.compatibilities.at(Tile::terrain_foot);

		bool connects[3][3];
		bool underWall = neighbours(wall, 0, 0, 0) || neighbours(foot, 0, 0, 0);

		if (relayUpdate && !underWall) {
			for (auto [xOffset, yOffset]: allNeighbours) {
				connects[xOffset+1][yOffset+1] = neighbours(top, xOffset, yOffset, 0) || neighbours(wall, xOffset, yOffset, 0);
			}
		}
		else {
			for (auto [xOffset, yOffset]: allNeighbours) {
				connects[xOffset+1][yOffset+1] = neighbours(top, xOffset, yOffset, 0);
			}
		}
	
		/* Connection mapping:
		 * 00 10 20
		 * 01 [] 21
		 * 02 12 22
		 */

		SubTile::Pattern p_tl, p_tr, p_bl, p_br;
		p_tl = getPattern_PathLike(connects[0][1], connects[1][0], connects[0][0]);
		p_tr = getPattern_PathLike(connects[2][1], connects[1][0], connects[2][0]);
		p_bl = getPattern_PathLike(connects[0][1], connects[1][2], connects[0][2]);
		p_br = getPattern_PathLike(connects[2][1], connects[1][2], connects[2][2]);

		if (!underWall) {
			if (p_tl == p_tr && p_tr == p_bl && p_bl == p_br) {
			tile.subTiles.push_back(set.getSubTile(tile, p_tl, SubTile::full));
			}
			else {
				tile.subTiles.push_back(set.getSubTile(tile, p_tl, SubTile::tlCorner));
				tile.subTiles.push_back(set.getSubTile(tile, p_tr, SubTile::trCorner));
				tile.subTiles.push_back(set.getSubTile(tile, p_bl, SubTile::blCorner));
				tile.subTiles.push_back(set.getSubTile(tile, p_br, SubTile::brCorner));
			}
		}
		else {
			auto shouldSpread = [&](int corner_xOffset, int corner_yOffset) {
				if (neighbours(top, corner_xOffset, 0, 0)) {
					if (!neighbours(wall, corner_xOffset, 0, 0) && !neighbours(foot, corner_xOffset, 0, 0))
						return true;
					if (neighbours(top, 0, corner_yOffset, 0)) {
						if (neighbours(top, corner_xOffset, corner_yOffset, 0)
						&& !neighbours(wall, corner_xOffset, corner_yOffset, 0)
						&& !neighbours(foot, corner_xOffset, corner_yOffset, 0))
							return true;
					}
					
				}
				if (neighbours(top, 0, corner_yOffset, 0)) {
					if (!neighbours(wall, 0, corner_yOffset, 0) && !neighbours(foot, 0, corner_yOffset, 0))
						return true;
				}
				return false;
			};

			if (shouldSpread(-1, -1)) tile.subTiles.push_back(set.getSubTile(tile, p_tl, SubTile::tlCorner));
			if (shouldSpread(1, -1)) tile.subTiles.push_back(set.getSubTile(tile, p_tr, SubTile::trCorner));
			if (shouldSpread(-1, 1)) tile.subTiles.push_back(set.getSubTile(tile, p_bl, SubTile::blCorner));
			if (shouldSpread(1, 1)) tile.subTiles.push_back(set.getSubTile(tile, p_br, SubTile::brCorner));	
		}

		scene->setTile(tile, x, y, z, subz);

		if (relayUpdate) {
			if (!underWall) {
				for (auto [xOffset, yOffset]: allNeighbours) {
					if (neighbours(wall, xOffset, yOffset, 0)) {
						setFilledTile(top, x + xOffset, y + yOffset, z, false);
					}
				}
				for (auto [xOffset, yOffset]: allNeighbours) {
					if (neighbours(wall, xOffset, yOffset, 0)) {
						setFilledTile(foot, x + xOffset, y + yOffset, z, false);
					}
				}
			}

			for (auto [xOffset, yOffset]: allNeighbours) {
				updatePotentialNeighbour(foot, xOffset, yOffset, 0, false);
				updatePotentialNeighbour(top, xOffset, yOffset, 0, false);
			}
		}
		break;
	}
	default: break;
	}
}

//Analyses connections to return the proper corner pattern
SubTile::Pattern SceneEditionTool::getPattern_PathLike(bool hConnection, bool vConnection, bool cConnection) {
	if (hConnection) {
		if (vConnection) {
			if (cConnection)
				return SubTile::center;
			return SubTile::cross;
		}
		return SubTile::horizontal;
	}
	else {
		if (vConnection)
			return SubTile::vertical;
		return SubTile::patch;
	}
}

Tile const* SceneEditionTool::checkName(int x, int y, int z, std::string const& filter) const {
	Tile const* t = scene->getTile(x, y, z, preferredSubZ.at(scene->getTileSet().getCategory(filter)));
	if (t == nullptr)
		return t;
	return t->info->name == filter ? t : nullptr;
}

bool TerrainTool::use(int x, int y, int z) const {
	int minHeight = scene->getHighestTileHeight(x, y, 0, z);
	minHeight = std::max(minHeight, lowestHeight);

	if (z < minHeight)
		return false;

	for (int h = minHeight; h < z; h++) {
		setFilledTile(wall, x, y, h);
	}

	setFilledTile(top, x, y, z);
	return true;
}