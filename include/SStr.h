#ifndef __STORMLIBX_SSTR_H__
#define __STORMLIBX_SSTR_H__

#include "StormTypes.h"
#include <cstdarg>


#define SSTR_UNBOUNDED 0x7FFFFFFF


// @501
DWORD STORMAPI SStrCopy(LPTSTR dest, LPCTSTR source, DWORD destsize = SSTR_UNBOUNDED);

// @502
//DWORD STORMAPI SStrHash(LPCTSTR string, DWORD flags = 0, DWORD seed = 0);

// @503
void STORMAPI SStrPack(LPTSTR dest, LPCTSTR source, DWORD destsize = SSTR_UNBOUNDED);

// @505
LPTSTR STORMAPI SStrChr(LPTSTR string, CHAR ch, BOOL reversed);

// @506
DWORD STORMAPI SStrLen(LPCTSTR string);

// @508
int STORMAPI SStrCmp(LPCTSTR string1, LPCTSTR string2, DWORD size);

// @509
int STORMAPI SStrCmpI(LPCTSTR string1, LPCTSTR string2, DWORD size);

// @510
LPTSTR STORMAPI SStrUpper(LPTSTR string);


// @569
LPTSTR STORMAPI SStrChrF(LPTSTR string, CHAR ch);

// @570
LPTSTR STORMAPI SStrChrR(LPTSTR string, CHAR ch);

// @571
LPCTSTR STORMAPI SStrChrF(LPCTSTR string, CHAR ch);

// @572
LPCTSTR STORMAPI SStrChrR(LPCTSTR string, CHAR ch);

// @578
DWORD __cdecl SStrPrintf(LPSTR dest, DWORD destsize, LPCSTR format, ...);

// @581
DWORD __cdecl SStrVPrintf(LPSTR dest, DWORD destsize, LPCSTR format, std::va_list args);

// @584, @585
LPCTSTR STORMAPI SStrStr(LPCTSTR string, LPCTSTR substring);


#endif
