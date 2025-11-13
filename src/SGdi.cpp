#include "StormTypes.h"
#include "SGdi.h"
#include "SDraw.h"
#include <vector>
#include "SMem.h"

#define SIGNATURE 0x4F4D    // 'M' 'O'
#define TYPE_FONT 0

static std::int32_t s_screencx;
static std::int32_t s_screency;
static std::int32_t s_screenbpp;
static std::int32_t s_pitch;
static BOOL s_screenbppshift;
static std::vector<std::int32_t> s_pitchtable;

static HSGDIOBJ s_selected;

static STORM_LIST(SGDIOBJ) s_objectlist;


class SGDIOBJ : public TSLinkedNode<SGDIOBJ> {
public:
  std::uint16_t signature;
  std::uint16_t type;
  void* bits;
};

class SGDIFONT : public SGDIOBJ {
public:
  std::int32_t filecharwidth;
  std::int32_t filecharheight;
  std::int32_t filepitch;
  std::int32_t filecolumnand;
  std::int32_t filerowshift;
  SIZE charsize[256];
};


// @383
BOOL STORMAPI SGdiDeleteObject(HSGDIOBJ handle) {
  if (!handle) return FALSE;
  if (handle->signature != SIGNATURE) return FALSE;
  if (handle->type != TYPE_FONT) return FALSE;

  if (s_selected == handle) s_selected = nullptr;

  FREEPTRIFUSED(handle->bits);
  s_objectlist.DeleteNode(handle);
  return TRUE;
}

// @385
BOOL STORMAPI SGdiExtTextOut(std::uint8_t* videobuffer, int x, int y, RECT* rect, std::uint32_t color, int textcoloruse, int bkgcoloruse, const char* string, int chars) {
  return TRUE;
}

// @386
BOOL STORMAPI SGdiImportFont(HFONT windowsfont, HSGDIFONT *handle) {
  return TRUE;
}

// @389
BOOL STORMAPI SGdiSelectObject(HSGDIOBJ handle) {
  if (!handle || handle->signature != SIGNATURE || handle->type != TYPE_FONT) {
    return FALSE;
  }

  s_selected = handle;
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
  if (size) {
    size->cx = 0;
    size->cy = 0;
  }
  STORM_VALIDATE_BEGIN;
  STORM_VALIDATE(string);
  STORM_VALIDATE(size);
  STORM_VALIDATE_END;

  HSGDIFONT font = static_cast<HSGDIFONT>(s_selected);
  if (!font) return FALSE;

  if (chars < 0) {
    chars = SStrLen(string);
  }

  for (int i = 0; i < chars; i++) {
    size->cx += font->charsize[string[i]].cx;
    size->cy = std::max(size->cy, font->charsize[string[i]].cy);
  }
  return TRUE;
}
