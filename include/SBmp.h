#ifndef __STORMLIBX_SBMP_H__
#define __STORMLIBX_SBMP_H__

#include "StormTypes.h"


#define SBMP_IMAGETYPE_NONE 0
#define SBMP_IMAGETYPE_BMP  1
#define SBMP_IMAGETYPE_PCX  2
#define SBMP_IMAGETYPE_TGA  3

typedef void*(STORMAPI* SBMPALLOCPROC)(std::uint32_t);

extern "C" {
  // @321
  BOOL STORMAPI SBmpDecodeImage(std::uint32_t imagetype, LPBYTE imagedata, std::uint32_t imagebytes, LPPALETTEENTRY paletteentries, LPBYTE bitmapbits, std::uint32_t buffersize, int* width = NULL, int* height = NULL, int* bitdepth = NULL);

  // @323
  BOOL STORMAPI SBmpLoadImage(const char* filename, LPPALETTEENTRY paletteentries, LPBYTE bitmapbits, std::uint32_t buffersize, int* width = NULL, int* height = NULL, int* bitdepth = NULL);

  // @324
  BOOL STORMAPI SBmpSaveImage(const char* filename, LPPALETTEENTRY paletteentries, LPBYTE bitmapbits, int width, int height, int bitdepth);

  // @325
  BOOL STORMAPI SBmpAllocLoadImage(const char* filename, LPPALETTEENTRY paletteentries, LPBYTE* returnedbuffer, int* width = NULL, int* height = NULL, int* bitdepth = NULL, int requestedbitdepth = 0, SBMPALLOCPROC allocproc = NULL);

  // @326
  BOOL STORMAPI SBmpSaveImageEx(const char* filename, LPPALETTEENTRY paletteentries, LPBYTE bitmapbits, int width, int height, int bitdepth, int alignment);

  // @327
  BOOL STORMAPI SBmpPadImage(LPBYTE imagedata, std::uint32_t imagebytes, int width, int height, int alignment, std::uint32_t* out_newwidth);

  // @328
  int STORMAPI SBmpGetPitchForAlignment(int width, int align);
}

#endif
