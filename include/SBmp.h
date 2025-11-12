#ifndef __STORMLIBX_SBMP_H__
#define __STORMLIBX_SBMP_H__

#include "StormTypes.h"


#define SBMP_IMAGETYPE_NONE 0
#define SBMP_IMAGETYPE_BMP  1
#define SBMP_IMAGETYPE_PCX  2
#define SBMP_IMAGETYPE_TGA  3

typedef LPVOID(STORMAPI* SBMPALLOCPROC)(DWORD);

extern "C" {
  // @321
  BOOL STORMAPI SBmpDecodeImage(DWORD imagetype, LPBYTE imagedata, DWORD imagebytes, LPPALETTEENTRY paletteentries, LPBYTE bitmapbits, DWORD buffersize, int* width = NULL, int* height = NULL, int* bitdepth = NULL);

  // @323
  BOOL STORMAPI SBmpLoadImage(LPCSTR filename, LPPALETTEENTRY paletteentries, LPBYTE bitmapbits, DWORD buffersize, int* width = NULL, int* height = NULL, int* bitdepth = NULL);

  // @324
  BOOL STORMAPI SBmpSaveImage(LPCSTR filename, LPPALETTEENTRY paletteentries, LPBYTE bitmapbits, int width, int height, int bitdepth);

  // @325
  BOOL STORMAPI SBmpAllocLoadImage(LPCSTR filename, LPPALETTEENTRY paletteentries, LPBYTE* returnedbuffer, int* width = NULL, int* height = NULL, int* bitdepth = NULL, int requestedbitdepth = 0, SBMPALLOCPROC allocproc = NULL);

  // @326
  BOOL STORMAPI SBmpSaveImageEx(LPCSTR filename, LPPALETTEENTRY paletteentries, LPBYTE bitmapbits, int width, int height, int bitdepth, int alignment);

  // @327
  BOOL STORMAPI SBmpRealignImage(LPBYTE imagedata, DWORD imagebytes, int width, int height, int alignment, DWORD* out_newwidth);

  // @328
  int STORMAPI SBmpGetAligned(int width, int align);
}

#endif
