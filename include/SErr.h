#ifndef __STORMLIBX_SERR_H__
#define __STORMLIBX_SERR_H__

#include <Storm/Error.hpp>

#ifndef VALIDATEBEGIN
#define VALIDATEBEGIN STORM_VALIDATE_BEGIN
#define VALIDATE STORM_VALIDATE
#define VALIDATEEND STORM_VALIDATE_END
#define VALIDATEENDVOID STORM_VALIDATE_END_VOID
#endif

typedef void(__fastcall *RECEIVEERRORPROC)(LPCSTR, LPCSTR, LPCSTR*, DWORD, DWORD, bool);


void SErrSetBlizzardErrorFunction(RECEIVEERRORPROC pFn);

BOOL STORMAPI SErrGetErrorStr(DWORD errorcode, LPSTR buffer, DWORD bufferchars);

#endif
