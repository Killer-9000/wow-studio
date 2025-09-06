#pragma once

#include "data/DBCFile.h"
#include "data/CommonFileTypes.h"

struct Entity;

struct ADT
{
	struct MVERChunk : public SChunk
	{
		uint32_t version;
	};

	struct MHDRChunk : public SChunk
	{
		uint32_t flags; // 1 for MFBO
		uint32_t mcin;
		uint32_t mtex;
		uint32_t mmdx;
		uint32_t mmid;
		uint32_t mwmo;
		uint32_t mwid;
		uint32_t mddf;
		uint32_t modf;
		uint32_t mfbo;
		uint32_t mh2o;
		uint32_t mtxf;
		uint32_t padding[4];
	};

	struct MCINChunk : public SChunk
	{
		struct MCINEntry
		{
			uint32_t offset;
			uint32_t size;
			uint8_t pad[8];
		};

		MCINEntry entries[16][16];
	};

	struct MTEXChunk : public SChunk
	{
		char filenames[]; // Multiple zero-terminated strings.
	};

	struct MMDXChunk : public SChunk
	{
		char filenames[]; // Multiple zero-terminated strings.
	};

	struct MWMOChunk : public SChunk
	{
		char filenames[]; // Multiple zero-terminated strings.
	};

	struct MMIDChunk : public SChunk
	{
		uint32_t offsets[]; // Offset into MMDX
	};

	struct MWIDChunk : public SChunk
	{
		uint32_t offsets[]; // Offset into MWMO
	};

	struct MDDFChunk : public SChunk
	{
		enum MDDFFlags : uint16_t
		{
			MDDFFlags_Biodome = 0x1,
			MDDFFlags_Shrubbery = 0x2
		};

		struct MDDFEntry
		{
			uint32_t nameId; // References MMID
			uint32_t uniqueId;
			glm::vec3 position;
			glm::vec3 rotation;
			uint16_t scale; // Default 1024
			MDDFFlags flags;
		};

		MDDFEntry entries[];
	};

	struct MODFChunk : public SChunk
	{
		enum MODFFlags : uint16_t
		{
			MODFFlags_Destroyable = 0x1, // set for destroyable buildings like the tower in DeathknightStart. This makes it a server-controllable game object.
		};

		struct MODFEntry
		{
			uint32_t nameId; // References MWID
			uint32_t uniqueId;
			glm::vec3 position;
			glm::vec3 rotation;
			glm::vec3 extents[2];
			MODFFlags flags;
			uint16_t doodadSet; // References MODS
			uint16_t nameSet;
			uint8_t padding[2];
		};

		MODFEntry entries[];
	};

	struct MH20Chunk : public SChunk
	{
		struct MH20LiquidChunk
		{
			uint32_t offsetInstances;
			uint32_t layerCount;
			uint32_t offsetAttributes;
		} header[16 * 16];

		uint8_t data[]; // The structures below are in this.

		struct MH20ChunkAttributes
		{
			// 8x8 Bit Masks
			uint64_t fishable;
			uint64_t deep;
		};

		struct MH20LiquidInstance {
			uint16_t liquid_type;
			uint16_t LVF;
			float minHeightLevel;
			float maxHeightLevel;
			uint8_t xOffset;
			uint8_t yOffset;
			uint8_t width;
			uint8_t height;
			uint32_t offsetExistsBitmap;
			uint32_t offsetVertexData;
		};
	};

	struct MCNKChunk : public SChunk
	{
		enum MCNKFlags : uint32_t
		{
			MCNKFlags_HasMCSH = 0x1,
			MCNKFlags_Impass = 0x2,
			MCNKFlags_LqRiver = 0x4,
			MCNKFlags_LqOcean = 0x8,
			MCNKFlags_LqMagma = 0x10,
			MCNKFlags_LqSlime = 0x20,
			MCNKFlags_HasMCCV = 0x40,
			MCNKFlags_DoNotFixAlphaMap = 0x4000
		};

		MCNKFlags flags;
		uint32_t indexX;
		uint32_t indexY;
		uint32_t nLayer;
		uint32_t nDoodadRefs;
		uint32_t ofsHeight;
		uint32_t ofsNormal;
		uint32_t ofsLayer;
		uint32_t ofsRefs;
		uint32_t ofsAlpha;
		uint32_t sizeAlpha;
		uint32_t ofsShadow;  // MCNKFlags_HasMCSH
		uint32_t sizeShadow; // ^
		uint32_t areaid;
		uint32_t nMapObjRefs;
		uint16_t holesLowRes;
		uint16_t padding;
		uint8_t  reallyLowQualityTextureingMap[4][4];
		uint8_t  nEffectDoodad[2][2];
		uint32_t ofsSndEmitters;
		uint32_t nSndEmitters;
		uint32_t ofsLiquid;
		uint32_t sizeLiquid; // 8 when not used.
		glm::vec3 position;
		uint32_t ofsMCCV;  // MCNKFlags_HasMCCV
		uint32_t padding1;
		uint32_t padding2;
	};

	struct MFBOChunk : public SChunk
	{
		uint16_t minimum[3][3];
		uint16_t maximum[3][3];
	};

	struct MTXFChunk : public SChunk
	{
		// Same count as MTEX
		uint32_t flags[]; // Use cubemaps.
	};

	// MCNK Sub-Chunks
	struct MCVTChunk : public SChunk
	{
		float height[9 * 9 + 8 * 8];
	};

	struct MCCVChunk : public SChunk
	{
		glm::u8vec4 colours[9 * 9 + 8 * 8]; // BGRA
	};

	struct MCNRChunk : public SChunk
	{
		glm::i8vec3 normals[9 * 9 + 8 * 8];
		uint8_t padding[13];
	};

	struct MCLYChunk : public SChunk
	{
		struct MCLYFlags
		{
			uint32_t animationRotation : 3;
			uint32_t animationSpeed : 3;
			uint32_t animationEnabled : 1;
			uint32_t overbright : 1;
			uint32_t useAlphaMap : 1;
			uint32_t alphaMapCompressed : 1;
			uint32_t useCubeMapReflection : 1;
		};

		struct MCLYLayer
		{
			uint32_t textureId;
			MCLYFlags flags;
			uint32_t offsetInMCAL;
			uint32_t effectId;
		};

		MCLYLayer layers[4]; // Max amount

	};

	//struct MCRFChunk : public SChunk
	//{
	//	uint32_t doodadRefs[]; // nDoodadRefs, references MDDF
	//	uint32_t objectRefs[]; // nMapObjRefs, references MODF
	//};

	struct MCSHChunk : public SChunk
	{
		// 64 x 64 bits
		uint8_t shadowMap[512];
	};

	struct MCALChunk : public SChunk
	{
		enum class CompressionMode : uint8_t
		{
			copy = 0,
			fill = 1,
		};

		struct MCALCompressedData
		{
			uint8_t count : 7;
			CompressionMode mode : 1;
			uint8_t value[];
		};

		// Uncompressed 8bit x 4096, Uncompressed 4bit x 2048
		uint8_t alphaMap[4096];

	};

	struct MCLQChunk : public SChunk
	{
		union MCLQVert
		{
			struct WaterVert
			{
				uint8_t depth;
				uint8_t flow0Pct;
				uint8_t flow1Pct;
				uint8_t filler;
				float height;
			} waterVert;
			struct OceanVert
			{
				uint8_t depth;
				uint8_t foam;
				uint8_t wet;
				uint8_t filler;
			} oceanVert;
			struct MagmaVert
			{
				uint16_t s;
				uint16_t t;
				float height;
			} magmaVert;
		};

		struct MCLQTiles
		{
			uint8_t tiles[8][8];
		};

		struct MCLQFlow
		{
			glm::vec3 spherePos;
			float sphereSize;
			glm::vec3 dir;
			float velocity;
			float amplitude;
			float frequency;
		};

		glm::vec2 heightRange;
		MCLQVert verts[9 * 9];
		MCLQTiles tiles;
		uint32_t nFlowvs;
		MCLQFlow flow[2]; // always 2 in file, independent on nFlowvs.
	};

	struct MCSEChunk : public SChunk
	{
		uint32_t entryId;
		glm::vec3 position;
		uint8_t padding[8];
		glm::vec3 size;
		uint8_t padding2[8];
	};
};

class MapTileLoader
{
public:
	static void Load(void* gpuDataBuffer, Entity* tile, MapDBC::Record* record, uint32_t x, uint32_t y);
};
