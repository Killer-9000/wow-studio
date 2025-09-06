#pragma once

#include "IViewport.h"
#include "data/ClientData.h"
#include "data/WorldData.h"
#include <graphics/renderers/WorldRenderer.h>
#include "loaders/AsyncLoader.h"
#include "loaders/MapTileLoader.h"
#include "loaders/WorldDataLoader.h"

#include <fmt/printf.h>

// Handles view of an open world within a map.
class WorldView : public IViewport
{
public:
	WorldView()
	{
		worldRenderer.Upload();
	}

	~WorldView()
	{
		worldRenderer.Unload();
	}

	bool LoadMap(uint32_t mapID)
	{
		MapDBC::Record* record = SClientData.MapDatabase.GetRecord(mapID);

		if (!record)
			return false;


		// Load WDT
		if (!SArchiveMgr->FileExists(fmt::sprintf("World\\Maps\\%s\\%s.wdt", record->Directory, record->Directory)))
			return false;
		
		if (!WorldDataLoader::Load(&_worldData, record))
			return false;

		MapTileRenderer& renderer = worldRenderer.mapTileRenderer;

		// Load ADTs
		for (int x = 0; x < 64; x++)
			for (int y = 0; y < 64; y++)
			{
				if (SArchiveMgr->FileExists(fmt::sprintf("World\\Maps\\%s\\%s_%i_%i.adt", record->Directory, record->Directory, x, y)))
				{
					Entity* entity = &_mapTiles[x][y];
					*entity = CreateEntity(Entity::Type::MapTile);
					SAsyncLoader->AddWork([&renderer, entity, x, y, record]() { MapTileLoader::Load(renderer.gpuDataBuffer, entity, record, x, y); });
				}
			}

		return true;
	}

	bool LoadWMO(std::string filename)
	{
		return false;
	}

	bool LoadModel(std::string filename)
	{
		return false;
	}

	glm::vec3 GetCameraForward() const
	{
		glm::vec3 direction;
		direction.x = cos(glm::radians(CameraRot.y)) * cos(glm::radians(CameraRot.x));
		direction.y = sin(glm::radians(CameraRot.x));
		direction.z = sin(glm::radians(CameraRot.y)) * cos(glm::radians(CameraRot.x));
		return glm::normalize(direction);
	}
	glm::vec3 GetCameraRight() const
	{
		return glm::cross(GetCameraForward(), glm::vec3(0, 1, 0));
	}

	glm::vec3 CameraPos = glm::vec3(0.0f, 10.0f, 0.0f);
	glm::vec3 CameraRot = glm::vec3(0.0f, 0.0f, 0.0f);
	float CameraSpeed = 1.0f;
	glm::mat4 CameraMatrix;
	glm::mat4 ProjectionMatrix;
	SDL_GPUBuffer* CameraVPMatrix;
	WorldRenderer worldRenderer;

private:
	WorldData _worldData;
	
	Entity _mapTiles[64][64];
};
