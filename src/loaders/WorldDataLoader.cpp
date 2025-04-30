#include "WorldDataLoader.h"
#include "data/WorldData.h"

#include <fmt/printf.h>
#include <tsl/robin_map.h>

bool WorldDataLoader::Load(WorldData* worldData, MapDBC::Record* record)
{
	if (!worldData)
		return false;

	uint8_t* data = nullptr;
	uint32_t size = 0;
	std::string filename = fmt::sprintf("World\\Maps\\%s\\%s.wdt", record->Directory, record->Directory);
	if (!SArchiveMgr->OpenFile(filename, &data, &size))
		return false;

	if (!size)
	{
		printf("ADT '%s.wdt' is empty.\n", record->Directory.data());
		return false;
	}

	tsl::robin_map<uint32_t, SChunk*> chunks;
	chunks.reserve(13);

	uint8_t* ptr = data;
	while (ptr < data + size)
	{
		SChunk* chunk = (SChunk*)ptr;
		switch (chunk->magic.magicI)
		{
		case 'MVER': chunks.emplace('MVER', chunk); break;
		case 'MPHD': chunks.emplace('MPHD', chunk); break;
		case 'MAIN': chunks.emplace('MAIN', chunk); break;
		case 'MWMO': chunks.emplace('MWMO', chunk); break;
		case 'MODF': chunks.emplace('MODF', chunk); break;
		default: printf("WARN: Unknown ADT Chunk %c%c%c%c.", chunk->magic.magicC[0], chunk->magic.magicC[1], chunk->magic.magicC[2], chunk->magic.magicC[3]);
		}

		ptr += chunk->size + sizeof(SChunk);
	}

	WDT::MPHDChunk* mphd = (WDT::MPHDChunk*)chunks['MPHD'];
	WDT::MAINChunk* main = (WDT::MAINChunk*)chunks['MAIN'];

	worldData->UseMCCV = mphd->flags & WDT::MPHDChunk::MPHDFlags_HasMCCV;
	worldData->SortDoodadRefs = mphd->flags & WDT::MPHDChunk::MPHDFlags_HasDoodadRefsSortedBySizeCat;

	for (int x = 0; x < 64; x++)
		for (int y = 0; y < 64; y++)
			worldData->HasADT[x][y] = main->mapAreaInfo[x][y].Flag_HasADT;

	return true;
}
