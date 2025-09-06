#pragma once

#include "components/BaseComponent.h"
#include <glm/glm.hpp>

#define ADT_TILE_SIZE 533.33333f
#define ADT_MAX_WIDTH 64

struct MapTile : public BaseComponent
{
	struct GPUData
	{
		bool discard = true;
		bool hole[9 * 9 + 8 * 8];
		float height[9 * 9 + 8 * 8];
		glm::vec4 colour[9 * 9 + 8 * 8];
		glm::vec3 normal[9 * 9 + 8 * 8];
	};

	MapTile() = delete;
	MapTile(Entity* entity)
		: BaseComponent(entity)
	{
		memset(_gpuData, 0, sizeof(_gpuData));
	}

	// Chunk data for the GPU.
	GPUData _gpuData[16][16];
	bool _loaded = false;
};
