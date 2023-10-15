#include "SStr.h"

#include <storm/String.hpp>
#include <cstdarg>


// @501
DWORD STORMAPI SStrCopy(LPTSTR dest, LPCTSTR source, DWORD destsize) {
  return ImplWrapSStrCopy(dest, source, destsize);
}

// @503
void STORMAPI SStrPack(LPTSTR dest, LPCTSTR source, DWORD destsize) {
  ImplWrapSStrPack(dest, source, destsize);
}

// @506
DWORD STORMAPI SStrLen(LPCTSTR string) {
  return ImplWrapSStrLen(string);
}

// @508
int STORMAPI SStrCmp(LPCTSTR string1, LPCTSTR string2, DWORD size) {
  return ImplWrapSStrCmp(string1, string2, size);
}

// @509
int STORMAPI SStrCmpI(LPCTSTR string1, LPCTSTR string2, DWORD size) {
  return ImplWrapSStrCmpI(string1, string2, size);
}

// @510
LPTSTR STORMAPI SStrUpper(LPTSTR string) {
  ImplWrapSStrUpper(string);
  return string;
}


// @571
LPTSTR STORMAPI SStrChrF(LPTSTR string, CHAR ch) {
  return ImplWrapSStrChr(string, ch);
}

// @572
LPTSTR STORMAPI SStrChrR(LPTSTR string, CHAR ch) {
  return ImplWrapSStrChrR(string, ch);
}

// @573
LPCTSTR STORMAPI SStrChrF(LPCTSTR string, CHAR ch) {
  return ImplWrapSStrChr(string, ch);
}

// @574
LPCTSTR STORMAPI SStrChrR(LPCTSTR string, CHAR ch) {
  return ImplWrapSStrChrR(string, ch);
}

// @578
DWORD __cdecl SStrPrintf(LPSTR dest, DWORD destsize, LPCSTR format, ...) {
  std::va_list args;
  va_start(args, format);
  DWORD result = SStrVPrintf(dest, destsize, format, args);
  va_end(args);
  return result;
}

// @581
DWORD __cdecl SStrVPrintf(LPSTR dest, DWORD destsize, LPCSTR format, std::va_list args) {
  return ImplWrapSStrVPrintf(dest, destsize, format, args);
}

// @587
LPCTSTR STORMAPI SStrStr(LPCTSTR string, LPCTSTR substring) {
  return ImplWrapSStrStr(string, substring);
}
