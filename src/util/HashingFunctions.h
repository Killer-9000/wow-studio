#pragma once

#include <stdint.h>

uint32_t JenkinsHash(const char* s)
{
  uint32_t hash = 0;

  for (; *s; ++s)
  {
    hash += *s;
    hash += (hash << 10);
    hash ^= (hash >> 6);
  }

  hash += (hash << 3);
  hash ^= (hash >> 11);
  hash += (hash << 15);

  return hash;
}