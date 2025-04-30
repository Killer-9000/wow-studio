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
	//// Render map tiles.
	//SRenderer->GetDeviceContext()->DrawIndexedIndirect();
	mapTileRenderer.Render(view);
}
