#ifndef __STORMLIBX_SGDI_H__
#define __STORMLIBX_SGDI_H__

#include "StormTypes.h"

class SGDIOBJ;
class SGDIFONT;

typedef SGDIOBJ* HSGDIOBJ;
typedef SGDIFONT* HSGDIFONT;


// @383
BOOL STORMAPI SGdiDeleteObject(HSGDIOBJ handle);

// @385
BOOL STORMAPI SGdiExtTextOut(std::uint8_t* videobuffer, int x, int y, RECT* rect, std::uint32_t color, int textcoloruse, int bkgcoloruse, const char* string, int chars = -1);

// @386
BOOL STORMAPI SGdiImportFont(HFONT windowsfont, HSGDIFONT *handle);

// @389
BOOL STORMAPI SGdiSelectObject(HSGDIOBJ handle);

// @390
BOOL STORMAPI SGdiSetPitch(int pitch);

// @392
BOOL STORMAPI SGdiSetTargetDimensions(int width, int height, int bitdepth, int pitch);

// @393
BOOL STORMAPI SGdiGetTextExtent(const char* string, int chars, LPSIZE size);


#endif
