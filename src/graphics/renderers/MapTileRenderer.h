#pragma once

#include <entt/entt.hpp>
#include <Buffer.h>
#include <RefCntAutoPtr.hpp>

class MapTile;

class MapTileRenderer
{
public:
	void Upload();
	void Unload();
	void Render(entt::view<MapTile>& tile);

	Diligent::RefCntAutoPtr<Diligent::IBuffer> gpuDataBuffer;

private:
	Diligent::RefCntAutoPtr<Diligent::IBuffer> _vertexBuffer;
	Diligent::RefCntAutoPtr<Diligent::IBuffer> _indexBuffer;
};
