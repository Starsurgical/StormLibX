#ifndef __STORMLIBX_SMEM_H__
#define __STORMLIBX_SMEM_H__

#include <Storm/Memory.hpp>

#ifndef ALLOC
#define ALLOC(bytes)        SMemAlloc(bytes,__FILE__,__LINE__,0)
#define ALLOCZERO(bytes)    SMemAlloc(bytes,__FILE__,__LINE__,SMEM_FLAG_ZEROMEMORY)
#define FREE(ptr)           SMemFree(ptr,__FILE__,__LINE__,0)
#define FREEIFUSED(ptr)     {if (ptr) SMemFree(ptr,__FILE__,__LINE__,0);}
#define FREEPTR(ptr)        SMemFree((PVOID)ptr,__FILE__,__LINE__,0); ((ptr) = NULL)
#define FREEPTRIFUSED(ptr)  {if (ptr) {SMemFree(ptr,__FILE__,__LINE__,0); (ptr) = NULL;}}
#endif

#endif
