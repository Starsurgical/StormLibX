#ifndef __STORMLIBX_SVID_H__
#define __STORMLIBX_SVID_H__

#include "StormTypes.h"


class VIDEOREC;
typedef VIDEOREC* VIDEOPTR;


typedef struct _SVIDPALETTEUSE {
  std::uint32_t size;
  std::uint32_t firstentry;
  std::uint32_t numentries;
} SVIDPALETTEUSE, * SVIDPALETTEUSEPTR;


#define SVID_FLAG_DOUBLESCANS       0x00000001
#define SVID_FLAG_INTERPOLATE       0x00000002
#define SVID_FLAG_AUTOQUALITY       0x00000008
#define SVID_FLAG_1XSIZE            0x00000100
#define SVID_FLAG_2XSIZE            0x00000200
#define SVID_FLAG_AUTOSIZE          0x00000800
#define SVID_FLAG_FILEHANDLE        0x00010000
#define SVID_FLAG_PRELOAD           0x00020000
#define SVID_FLAG_LOOP              0x00040000
#define SVID_FLAG_FULLSCREEN        0x00080000
#define SVID_FLAG_USECURRENTPALETTE 0x00100000
#define SVID_FLAG_CLEARSCREEN       0x00200000
#define SVID_FLAG_NOSKIP            0x00400000
#define SVID_FLAG_UNK               0x00800000
#define SVID_FLAG_UNK2              0x01000000
#define SVID_FLAG_TOSCREEN          0x10000000
#define SVID_FLAG_TOBUFFER          0x20000000

#define SVID_CUTSCENE       (SVID_FLAG_TOSCREEN | SVID_FLAG_FULLSCREEN | SVID_FLAG_CLEARSCREEN | SVID_FLAG_2XSIZE)
#define SVID_AUTOCUTSCENE   (SVID_FLAG_TOSCREEN | SVID_FLAG_FULLSCREEN | SVID_FLAG_CLEARSCREEN | SVID_FLAG_AUTOSIZE | SVID_FLAG_AUTOQUALITY)
#define SVID_CTRL           (SVID_FLAG_TOBUFFER | SVID_FLAG_UNK2 | SVID_FLAG_NOSKIP | SVID_FLAG_USECURRENTPALETTE | SVID_FLAG_PRELOAD | SVID_FLAG_1XSIZE)

#define SVID_QUALITY_LOW_SKIPSCANS       SVID_FLAG_2XSIZE
#define SVID_QUALITY_LOW                 (SVID_FLAG_2XSIZE | SVID_FLAG_DOUBLESCANS)
#define SVID_QUALITY_HIGH_SKIPSCANS      (SVID_FLAG_2XSIZE | SVID_FLAG_INTERPOLATE)
#define SVID_QUALITY_HIGH                (SVID_FLAG_2XSIZE | SVID_FLAG_INTERPOLATE | SVID_FLAG_DOUBLESCANS)


// @451
BOOL STORMAPI SVidDestroy();

// @452
BOOL STORMAPI SVidGetSize(VIDEOPTR video, int32_t *width, int32_t *height, int32_t *bitdepth = NULL);

// @453
BOOL STORMAPI SVidInitialize(void* directsound);

// @454
BOOL STORMAPI SVidPlayBegin(const char* filename, void* destbuffer, const RECT* destrect, SIZE* destsize, SVIDPALETTEUSEPTR paletteuse, uint32_t flags, VIDEOPTR *handle);

// @455
BOOL STORMAPI SVidPlayBeginFromMemory(const void *sourceptr, uint32_t sourcebytes, void* destbuffer, const RECT* destrect, SIZE* destsize, SVIDPALETTEUSEPTR paletteuse, uint32_t flags, VIDEOPTR *handle);

// @457
BOOL STORMAPI SVidPlayContinueSingle(VIDEOPTR video, BOOL forceupdate = FALSE, BOOL *updated = NULL);

// @458
BOOL STORMAPI SVidPlayEnd(VIDEOPTR video);


#endif
