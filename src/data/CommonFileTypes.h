#pragma once

#include <glm/glm.hpp>

union SMagic
{
	uint32_t magicI;
	char magicC[4];
};

struct IChunk
{
	SMagic magic;
	uint32_t size;
};
