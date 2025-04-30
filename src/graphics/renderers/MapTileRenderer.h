#pragma once

#include <entt/entt.hpp>
#include <SDL3/SDL.h>

class WorldView;

class MapTileRenderer
{
public:
	void Upload();
	void Unload();
	void Render(WorldView* view);

	SDL_GPUBuffer* gpuDataBuffer = nullptr;

private:
	SDL_GPUBuffer* _vertexBuffer = nullptr;
	SDL_GPUBuffer* _indexBuffer = nullptr;

	SDL_GPUGraphicsPipeline* _pipeline = nullptr;
};
