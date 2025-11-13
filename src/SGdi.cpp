#include "SGdi.h"
#include "SDraw.h"
#include <vector>


static std::int32_t s_screencx;
static std::int32_t s_screency;
static std::int32_t s_screenbpp;
static std::int32_t s_pitch;
static BOOL s_screenbppshift;
static std::vector<std::int32_t> s_pitchtable;


// @383
BOOL STORMAPI SGdiDeleteObject(HSGDIOBJ handle) {
  return TRUE;
}

// @385
BOOL STORMAPI SGdiExtTextOut(std::uint8_t* videobuffer, int x, int y, RECT* rect, std::uint32_t color, int textcoloruse, int bkgcoloruse, const char* string, int chars = -1) {
  return TRUE;
}

// @386
BOOL STORMAPI SGdiImportFont(HFONT windowsfont, HSGDIFONT *handle) {
  return TRUE;
}

// @389
BOOL STORMAPI SGdiSelectObject(HSGDIOBJ handle) {
  return TRUE;
}

// @390
BOOL STORMAPI SGdiSetPitch(int pitch) {
  STORM_VALIDATE_BEGIN;
  STORM_VALIDATE(pitch > 0);
  STORM_VALIDATE_END;

  SDrawGetScreenSize(&s_screencx, &s_screency, &s_screenbpp);
  return SGdiSetTargetDimensions(s_screencx, s_screency, s_screenbpp, pitch);
}

// @392
BOOL STORMAPI SGdiSetTargetDimensions(int width, int height, int bitdepth, int pitch) {
  s_screencx = width;
  s_screency = height;
  s_screenbpp = bitdepth;
  s_screenbppshift = bitdepth == 16;

  if (pitch != s_pitch || height != s_pitchtable.size()) {
    s_pitch = pitch;

    s_pitchtable.clear();
    int offset = 0;
    for (int i = 0; i < height; offset += pitch) {
      s_pitchtable.push_back(offset);
    }
  }

  return TRUE;
}

// @393
BOOL STORMAPI SGdiGetTextExtent(const char* string, int chars, LPSIZE size) {
  return TRUE;
}
