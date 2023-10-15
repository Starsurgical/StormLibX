#include "SComp.h"

#include <StormLib.h>


// @551
BOOL STORMAPI SCompCompress(void* dest, int* destsize, void* source, int sourcesize, unsigned compressiontypes, int hint, int optimization) {
  return ImplWrapSCompCompress(dest, destsize, source, sourcesize, compressiontypes, hint, optimization);
}

// @552
BOOL STORMAPI SCompDecompress(void* dest, int* destsize, void* source, int sourcesize) {
  return ImplWrapSCompDecompress(dest, destsize, source, sourcesize);
}
