#pragma once
#include "TileSet.h"
#include "Scene.h"

class SceneEditionTool {
public:
	void setScene(Scene& scene);

	virtual bool use(int x, int y, int z) const = 0;

protected:
	void setFilledTile(std::string const& name, int x, int y, int z, bool relayUpdate = true) const;

	static SubTile::Pattern getPattern_PathLike(bool hConnection, bool vConnection, bool cConnection);

	Tile const* checkName(int x, int y, int z, std::string const& filter) const;

	Scene* scene;
};

class TerrainTool : public SceneEditionTool {
public:
	std::string top;
	std::string wall;
	std::string foot;
	int lowestHeight = 0;

	virtual bool use(int x, int y, int z) const;

};