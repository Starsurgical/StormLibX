#include "SVid.h"
#include "StormTypes.h"


typedef struct _SCODEEXECUTEDATA {
    uint32_t  size;
    uint32_t  flags;
    int32_t   xiterations;
    int32_t   yiterations;
    int32_t   adjustdest;
    int32_t   adjustsource;
    void*     dest;
    void*     source;
    void*     table;
    uint32_t  a;
    uint32_t  b;
    uint32_t  c;
    int32_t   adjustdestalt;
    int32_t   adjustsourcealt;
    uint32_t  reserved[2];
} SCODEEXECUTEDATA, *SCODEEXECUTEDATAPTR;

typedef struct _SMACKERDATA {
    void*   buffer;
    void*   file;
    int32_t top;
} SMACKERDATA, *SMACKERDATAPTR;

typedef class VIDEOREC : public TSLinkedNode<VIDEOREC> {
public:
  HANDLE           filehandle;
  SCODEEXECUTEDATA executedata;
  uint8_t*         interptable;
  uint32_t         palettefirstentry;
  uint32_t         palettenumentries;
  SMACKERDATAPTR   smackerdata;
  uint8_t*         destbuffer;
  RECT             destrect;
  const RECT*      ddrect;
  SIZE             destsize;
  uint8_t*         lockedbuffer;
  uint32_t         flags;
  BOOL             vertzoom;
  BOOL             vertinterp;
  int32_t          altline;
  HSRGN            rgn[2];
  uint32_t         allocrects;
  uint32_t         numrects;
  RECT*            rect;
} *VIDEOPTR;


static STORM_LIST(VIDEOREC) s_videolist;


// @451
BOOL STORMAPI SVidDestroy() {
  while (VIDEOREC* video = s_videolist.Head()) {
    SVidPlayEnd(video);
  }

  // TODO clean up smacker lib here

  return TRUE;
}

// @452
BOOL STORMAPI SVidGetSize(VIDEOPTR video, int32_t *width, int32_t *height, int32_t *bitdepth) {
  return TRUE;
}

// @453
BOOL STORMAPI SVidInitialize(void* directsound) {
  // TODO initialize smacker lib here
  return TRUE;
}

// @454
BOOL STORMAPI SVidPlayBegin(const char* filename, void* destbuffer, const RECT* destrect, SIZE* destsize, SVIDPALETTEUSEPTR paletteuse, uint32_t flags, VIDEOPTR *handle) {
  return FALSE;
}

// @455
BOOL STORMAPI SVidPlayBeginFromMemory(const void *sourceptr, uint32_t sourcebytes, void* destbuffer, const RECT* destrect, SIZE* destsize, SVIDPALETTEUSEPTR paletteuse, uint32_t flags, VIDEOPTR *handle) {
  return FALSE;
}

// @457
BOOL STORMAPI SVidPlayContinueSingle(VIDEOPTR video, BOOL forceupdate, BOOL *updated) {
  return FALSE;
}

// @458
BOOL STORMAPI SVidPlayEnd(VIDEOPTR video) {
  return TRUE;
}
