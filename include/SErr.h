#ifndef __STORMLIBX_SERR_H__
#define __STORMLIBX_SERR_H__

#include "StormTypes.h"
#include <Storm/Error.hpp>

#ifndef VALIDATEBEGIN
#define VALIDATEBEGIN STORM_VALIDATE_BEGIN
#define VALIDATE STORM_VALIDATE
#define VALIDATEEND STORM_VALIDATE_END
#define VALIDATEENDVOID STORM_VALIDATE_END_VOID
#endif

typedef void(__fastcall *RECEIVEERRORPROC)(const char*, const char*, const char**, std::uint32_t, std::uint32_t, bool);


void SErrSetBlizzardErrorFunction(RECEIVEERRORPROC pFn);

BOOL STORMAPI SErrGetErrorStr(std::uint32_t errorcode, char* buffer, std::uint32_t bufferchars);

#endif
