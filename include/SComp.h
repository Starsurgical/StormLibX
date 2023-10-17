#ifndef __STORMLIBX_SCOMP_H__
#define __STORMLIBX_SCOMP_H__

#include "StormTypes.h"

extern "C" {
  // @551
  BOOL STORMAPI SCompCompress(void* dest, int* destsize, void* source, int sourcesize, unsigned compressiontypes, int hint, int optimization);

  // @552
  BOOL STORMAPI SCompDecompress(void* dest, int* destsize, void* source, int sourcesize);
}

#endif
