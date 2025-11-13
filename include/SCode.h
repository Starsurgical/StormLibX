#ifndef __STORMLIBX_SCODE_H__
#define __STORMLIBX_SCODE_H__

#include "StormTypes.h"


#define  SCODE_CF_AUTOALIGNDWORD  0x00040000
#define  SCODE_CF_USESALTADJUSTS  0x04000000

DECLARE_HANDLE(HSCODESTREAM);

typedef struct _SCODEEXECUTEDATA {
  std::uint32_t  size;
  std::uint32_t  flags;
  int    xiterations;
  int    yiterations;
  int    adjustdest;
  int    adjustsource;
  void* dest;
  void* source;
  void* table;
  std::uint32_t  a;
  std::uint32_t  b;
  std::uint32_t  c;
  int    adjustdestalt;
  int    adjustsourcealt;
  std::uint32_t  reserved[2];
} SCODEEXECUTEDATA, * SCODEEXECUTEDATAPTR;


extern "C" {
}

#endif
