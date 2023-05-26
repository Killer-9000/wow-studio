#include "WorldRenderer.h"
#include "components/MapTile.h"
#include "views/WorldView.h"

void WorldRenderer::Upload()
{
	mapTileRenderer.Upload();
}

void WorldRenderer::Unload()
{
	mapTileRenderer.Unload();
}

void WorldRenderer::Render(WorldView* view)
{
	auto mapTileView = view->GetRegistry().view<MapTile>();

	// Check if rendering data needs changing.
	for (const auto& [entity, mapTile] : mapTileView.each())
	{
		if (!mapTile.loaded)
			continue;
	}

	//// Render map tiles.
	//SRenderer->GetDeviceContext()->DrawIndexedIndirect();
	mapTileRenderer.Render(mapTileView);
}
