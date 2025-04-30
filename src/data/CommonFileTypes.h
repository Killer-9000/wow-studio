#pragma once

#include <glm/glm.hpp>

union UMagic
{
	uint32_t magicI;
	char magicC[4];
};

struct SChunk
{
	UMagic magic;
	uint32_t size;
};
