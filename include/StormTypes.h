#ifndef __STORMLIBX_STORMTYPES_H__
#define __STORMLIBX_STORMTYPES_H__

#include <StormPort.h>
#include <storm/Array.hpp>
#include <storm/Handle.hpp>
#include <storm/Hash.hpp>
#include <storm/List.hpp>
#include <storm/Queue.hpp>
#include <storm/thread/CCritSect.hpp>

typedef int BOOL;
#define FALSE 0
#define TRUE 1

#ifndef STORMAPI
#define STORMAPI __stdcall
#endif


#endif
