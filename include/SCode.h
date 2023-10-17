#ifndef __STORMLIBX_SCODE_H__
#define __STORMLIBX_SCODE_H__

#include "StormTypes.h"


#define  SCODE_CF_AUTOALIGNDWORD  0x00040000
#define  SCODE_CF_USESALTADJUSTS  0x04000000

DECLARE_HANDLE(HSCODESTREAM);

typedef struct _SCODEEXECUTEDATA {
  DWORD  size;
  DWORD  flags;
  int    xiterations;
  int    yiterations;
  int    adjustdest;
  int    adjustsource;
  LPVOID dest;
  LPVOID source;
  LPVOID table;
  DWORD  a;
  DWORD  b;
  DWORD  c;
  int    adjustdestalt;
  int    adjustsourcealt;
  DWORD  reserved[2];
} SCODEEXECUTEDATA, * SCODEEXECUTEDATAPTR;


extern "C" {
}

#endif
