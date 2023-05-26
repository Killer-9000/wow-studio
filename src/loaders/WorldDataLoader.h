#pragma once

#include "data/DBCFile.h"
#include "data/CommonFileTypes.h"

struct WorldData;

struct WDT
{
	struct MVERChunk : public IChunk
	{
		uint32_t version;
	};

	struct MPHDChunk : public IChunk
	{
		enum MPHDFlags : uint32_t
		{
			MPHDFlags_UsesGlobalMapObj = 0x1,
			MPHDFlags_HasMCCV = 0x2,
			MPHDFlags_HasDoodadRefsSortedBySizeCat = 0x8
		};

		MPHDFlags flags;
		uint32_t padding[7];
	};

	struct MAINChunk : public IChunk
	{
		struct MAINAreaInfo
		{
			uint32_t Flag_HasADT : 1;
			uint32_t Flag_Loaded : 1;

			uint32_t padding;
		};

		MAINAreaInfo mapAreaInfo[64][64];
	};

	struct MWMOChunk : public IChunk
	{
		char filename[255];
	};

	struct MODFChunk : public IChunk
	{
		enum MODFFlags : uint16_t
		{
			MODFFlags_Destroyable = 0x1, // set for destroyable buildings like the tower in DeathknightStart. This makes it a server-controllable game object.
		};

		uint32_t nameId;
		uint32_t uniqueId;
		glm::vec3 position;
		glm::vec3 rotation;
		glm::vec3 extents[2];
		MODFFlags flags;
		uint16_t doodadSet;
		uint16_t nameSet;
		uint8_t padding[2];
	};
};

class WorldDataLoader
{
public:
	static bool Load(WorldData* worldData, MapDBC::Record* record);
};
