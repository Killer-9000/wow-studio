#include "MapTileLoader.h"
#include "data/Entity.h"

#include <fmt/printf.h>
#include <tsl/robin_map.h>
#include <graphics/Renderer.h>

void MapTileLoader::Load(Diligent::RefCntAutoPtr<Diligent::IBuffer> gpuDataBuffer, Entity* tile, MapDBC::Record* record, uint32_t x, uint32_t y)
{
	if (!tile || !record || x > 64 || y > 64)
		return;

	uint8_t* data = nullptr;
	uint32_t size = 0;
	std::string filename = fmt::sprintf("World\\Maps\\%s\\%s_%i_%i.adt", record->Directory, record->Directory, x, y);
	if (!SArchiveMgr->OpenFile(filename, &data, &size))
		return;

	if (!size)
	{
		printf("ADT '%s_%i_%i.adt' is empty.\n", record->Directory.data(), x, y);
		return;
	}

	tsl::robin_map<uint32_t, IChunk*> chunks;
	chunks.reserve(13);

	// Load chunks.
	{
		uint8_t* ptr = data;
		while (ptr < data + size)
		{
			IChunk* chunk = (IChunk*)ptr;
			switch (chunk->magic.magicI)
			{
			case 'MVER': chunks.emplace('MVER', chunk); break;
			case 'MHDR': chunks.emplace('MHDR', chunk); break;
			case 'MCIN': chunks.emplace('MCIN', chunk); break;
			case 'MTEX': chunks.emplace('MTEX', chunk); break;
			case 'MMDX': chunks.emplace('MMDX', chunk); break;
			case 'MWMO': chunks.emplace('MWMO', chunk); break;
			case 'MMID': chunks.emplace('MMID', chunk); break;
			case 'MWID': chunks.emplace('MWID', chunk); break;
			case 'MDDF': chunks.emplace('MDDF', chunk); break;
			case 'MODF': chunks.emplace('MODF', chunk); break;
			case 'MH2O': chunks.emplace('MH2O', chunk); break;
			case 'MFBO': chunks.emplace('MFBO', chunk); break;
			case 'MTXF': chunks.emplace('MTXF', chunk); break;
			case 'MCNK': break;
			default: printf("WARN: Unknown ADT Chunk %c%c%c%c.\n", chunk->magic.magicC[3], chunk->magic.magicC[2], chunk->magic.magicC[1], chunk->magic.magicC[0]);
			}

			ptr += chunk->size + sizeof(IChunk);
		}
	}

	ADT::MCINChunk* mcin = (ADT::MCINChunk*)chunks['MCIN'];
	for (int x = 0; x < 16; x++)
		for (int y = 0; y < 16; y++)
		{
			MapTile::GPUData& gpuData = tile->GetMapTile()->_gpuData[x][y];

			uint8_t* ptr = data + mcin->entries[x][y].offset;
			ADT::MCNKChunk* mcnk = (ADT::MCNKChunk*)ptr;

			ADT::MCVTChunk* mcvt = (ADT::MCVTChunk*)(data + mcnk->ofsHeight);
			ADT::MCCVChunk* mccv = (ADT::MCCVChunk*)(data + mcnk->ofsMCCV);
			ADT::MCNRChunk* mcnr = (ADT::MCNRChunk*)(data + mcnk->ofsNormal);
			ADT::MCSHChunk* mcsh = (ADT::MCSHChunk*)(data + mcnk->ofsShadow);

			memcpy(gpuData.height, mcvt->height, 9*9 + 8*8);
			for (int i = 0; i < 9 * 9 + 8 * 8; i++)
			{
				glm::u8vec4& col = mccv->entries[i].colour;
				gpuData.colour[i] = { col.b, col.g, col.r, col.a };
			}
			for (int i = 0; i < 9 * 9 + 8 * 8; i++)
				gpuData.normal[i] = mcnr->entries[i].normal;

			for (int i = 0; i < 9 * 9 + 8 * 8; i++)
				gpuData.shadow[i] = false;
			for (int i = 0; i < 9 * 9 + 8 * 8; i++)
				gpuData.hole[i] = false;

			gpuData.discard = false;
		}

	void* mapped;
	SRendererContext->MapBuffer(gpuDataBuffer, Diligent::MAP_WRITE, Diligent::MAP_FLAG_NONE, mapped);
	memcpy((uint8_t*)mapped + ((sizeof(MapTile::GPUData) * (16 * 16)) * (y * 16 + x)), tile->GetMapTile()->_gpuData, sizeof(MapTile::_gpuData));
	SRendererContext->UnmapBuffer(gpuDataBuffer, Diligent::MAP_WRITE);

	tile->GetMapTile()->loaded = true;
}