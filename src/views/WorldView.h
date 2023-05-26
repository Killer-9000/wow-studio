#pragma once

#include "IViewport.h"
#include "data/WorldData.h"
#include "loaders/AsyncLoader.h"
#include "loaders/MapTileLoader.h"
#include "loaders/WorldDataLoader.h"

#include <Buffer.h>
#include <fmt/printf.h>
#include <RefCntAutoPtr.hpp>

// Handles view of an open world within a map.
class WorldView : public IViewport
{
public:
	WorldView(uint32_t mapID, WowLocaleEnum locale)
		: _mapID(mapID), _mapDBC(locale), _locale(locale)
	{
		_mapDBC.LoadFile();

		MapDBC::Record* record = _mapDBC.GetRecord(mapID);

		if (!record)
			return;

		worldRenderer.Upload();

		// Load WDT
		if (!SArchiveMgr->FileExists(fmt::sprintf("World\\Maps\\%s\\%s.wdt", record->Directory, record->Directory)))
			return;
		
		if (!WorldDataLoader::Load(&_worldData, record))
			return;

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
	}

	~WorldView()
	{
		worldRenderer.Unload();
	}

	WowLocaleEnum GetLocale() { return _locale; }

	glm::vec3 GetCameraForward()
	{
		glm::vec3 direction;
		direction.x = cos(glm::radians(CameraRot.y)) * cos(glm::radians(CameraRot.x));
		direction.y = sin(glm::radians(CameraRot.x));
		direction.z = sin(glm::radians(CameraRot.y)) * cos(glm::radians(CameraRot.x));
		return glm::normalize(direction);
	}
	glm::vec3 GetCameraRight()
	{
		return glm::cross(GetCameraForward(), glm::vec3(0, 1, 0));
	}

	glm::vec3 CameraPos = glm::vec3(0.0f, 10.0f, 0.0f);
	glm::vec3 CameraRot = glm::vec3(0.0f, 0.0f, 0.0f);
	float CameraSpeed = 1.0f;
	glm::mat4 CameraMatrix;
	glm::mat4 ProjectionMatrix;
	Diligent::RefCntAutoPtr<Diligent::IBuffer> CameraVPMatrix;
	WorldRenderer worldRenderer;

private:
	uint32_t _mapID;
	MapDBC _mapDBC;
	WowLocaleEnum _locale;
	WorldData _worldData;
	Entity _mapTiles[64][64];
};
