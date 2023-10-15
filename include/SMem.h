#ifndef __STORMLIBX_SMEM_H__
#define __STORMLIBX_SMEM_H__

#include "StormTypes.h"

#define SMEM_FLAG_ZEROMEMORY        0x00000008
#define SMEM_FLAG_PRESERVEONDESTROY 0x08000000

#define ALLOC(bytes)        SMemAlloc(bytes,__FILE__,__LINE__,0)
#define ALLOCZERO(bytes)    SMemAlloc(bytes,__FILE__,__LINE__,SMEM_FLAG_ZEROMEMORY)
#define FREE(ptr)           SMemFree(ptr,__FILE__,__LINE__,0)
#define FREEIFUSED(ptr)     {if (ptr) SMemFree(ptr,__FILE__,__LINE__,0);}
#define FREEPTR(ptr)        SMemFree((PVOID)ptr,__FILE__,__LINE__,0); ((ptr) = NULL)
#define FREEPTRIFUSED(ptr)  {if (ptr) {SMemFree(ptr,__FILE__,__LINE__,0); (ptr) = NULL;}}


// @401
LPVOID STORMAPI SMemAlloc(DWORD bytes, LPCSTR filename = NULL, int linenumber = 0, DWORD flags = 0);

// @403
BOOL STORMAPI SMemFree(LPVOID ptr, LPCSTR filename = NULL, int linenumber = 0, DWORD flags = 0);

// @405
LPVOID STORMAPI SMemReAlloc(LPVOID ptr, DWORD bytes, LPCSTR filename = NULL, int linenumber = 0, DWORD flags = 0);


// @491
void STORMAPI SMemCopy(void* dest, const void* source, size_t size);

// @492
void STORMAPI SMemFill(void* location, size_t length, BYTE fillWith = 0);

// @493
void STORMAPI SMemMove(void* dest, const void* source, size_t size);

// @494
void STORMAPI SMemZero(void* location, size_t length);

// @495
int STORMAPI SMemCmp(void* location1, void* location2, DWORD size);


#endif
