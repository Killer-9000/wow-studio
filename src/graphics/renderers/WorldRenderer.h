#pragma once

#include "MapTileRenderer.h"

class WorldView;

class WorldRenderer
{
public:
	void Upload();
	void Unload();
	void Render(WorldView* view);

	MapTileRenderer mapTileRenderer;

private:
};
