#include "MapTileLoader.h"
#include "data/Entity.h"
#include "graphics/Rendering.h"

#include <fmt/printf.h>
#include <tsl/robin_map.h>

void MapTileLoader::Load(void* gpuDataBuffer, Entity* tile, MapDBC::Record* record, uint32_t x, uint32_t y)
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

	tsl::robin_map<uint32_t, SChunk*> chunks;
	chunks.reserve(13);

	// Load chunks.
	{
		uint8_t* ptr = data;
		while (ptr < data + size)
		{
			SChunk* chunk = (SChunk*)ptr;
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

			ptr += chunk->size + sizeof(SChunk);
		}
	}

	ADT::MCINChunk* mcin = (ADT::MCINChunk*)chunks['MCIN'];
	for (int x = 0; x < 16; x++)
	{
		for (int y = 0; y < 16; y++)
		{
			MapTile::GPUData& gpuData = tile->GetMapTile()->_gpuData[x][y];

			uint8_t* ptr = data + mcin->entries[x][y].offset;
			ADT::MCNKChunk* mcnk = (ADT::MCNKChunk*)ptr;

			ADT::MCVTChunk* mcvt = (ADT::MCVTChunk*)(ptr + mcnk->ofsHeight);
			ADT::MCCVChunk* mccv = (ADT::MCCVChunk*)(mcnk->flags & ADT::MCNKChunk::MCNKFlags_HasMCCV ? ptr + mcnk->ofsMCCV : nullptr);
			ADT::MCNRChunk* mcnr = (ADT::MCNRChunk*)(ptr + mcnk->ofsNormal);
			ADT::MCSHChunk* mcsh = (ADT::MCSHChunk*)(mcnk->flags & ADT::MCNKChunk::MCNKFlags_HasMCSH ? ptr + mcnk->ofsShadow : nullptr);

			memcpy(gpuData.height, mcvt->height, 9 * 9 + 8 * 8);
			memcpy(gpuData.normal, mcnr->normals, 9 * 9 + 8 * 8);

			if (mccv)
			{
				for (int i = 0; i < 9 * 9 + 8 * 8; i++)
				{
					glm::u8vec4& col = mccv->colours[i];
					gpuData.colour[i] = { col.b, col.g, col.r, col.a };
				}
			}

			if (mcsh)
			{
				// TODO: This is more like a texture.
				// for (int i = 0; i < 9 * 9 + 8 * 8; i++)
				// {
				// 	gpuData.shadow[i] = mcsh->shadowMap;
				// }
			}

			// TODO: Once I figure out the triangle rendering method, this should be per-triangle, instead of per-vertex.
			for (int x = 0, i = 0; x < 4; x++)
			for (int y = 0;        y < 4; y++, i++)
			{
				bool hole = mcnk->holesLowRes & 1 << i;
				
				//  0     1     2
				//     9    10
				// 17    18    19
				//    26    27
				// 34    35    36

				// x x x
				//  x x
				// x x x
				//  x x
				// x x x
				
				gpuData.hole[0] = false;
				gpuData.hole[1] = false;
				gpuData.hole[2] = false;
				
				gpuData.hole[y * 34 + x * 8 + 9] = hole;
				gpuData.hole[y * 34 + x * 8 + 10] = hole;

				gpuData.hole[17] = false;
				gpuData.hole[18] = false;
				gpuData.hole[19] = false;
				
				gpuData.hole[y * 34 + x * 8 + 26] = hole;
				gpuData.hole[y * 34 + x * 8 + 27] = hole;
				
				gpuData.hole[34] = false;
				gpuData.hole[35] = false;
				gpuData.hole[36] = false;
			}

			gpuData.discard = false;
		}
	}

	//Rendering::UploadBufferData(device, gpuDataBuffer, );

	//SDL_CreateGPUTransferBuffer();
	//SDL_MapGPUTransferBuffer();
	//SDL_UnmapGPUTransferBuffer();
	//SDL_UploadToGPUBuffer();
	//SDL_ReleaseGPUTransferBuffer();

	//void* mapped;
	//SRendererContext->MapBuffer(gpuDataBuffer, Diligent::MAP_WRITE, Diligent::MAP_FLAG_NONE, mapped);
	//memcpy((uint8_t*)mapped + ((sizeof(MapTile::GPUData) * (16 * 16)) * (y * 16 + x)), tile->GetMapTile()->_gpuData, sizeof(MapTile::_gpuData));
	//SRendererContext->UnmapBuffer(gpuDataBuffer, Diligent::MAP_WRITE);

	tile->GetMapTile()->_loaded = true;
}