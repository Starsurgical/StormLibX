#include "SErr.h"

#include <storm/Error.hpp>
#include <StormLib.h>
#include <cstdarg>

void* s_err_callback;


void SErrSetBlizzardErrorFunction(void* pFn) {
  s_err_callback = pFn;
}

// @461
BOOL STORMAPI SErrDisplayError(DWORD errorcode, LPCTSTR filename, int linenumber, LPCTSTR description, BOOL recoverable, DWORD exitcode) {
  return ImplWrapSErrDisplayError(errorcode, filename, linenumber, description, recoverable, exitcode, 0);
}

// @463
DWORD STORMAPI SErrGetLastError() {
  DWORD err = GetLastError();
  if (err == ERROR_SUCCESS) {
    err = ImplWrapSErrGetLastError();
  }
  return err;
}

// @465
void STORMAPI SErrSetLastError(DWORD errorcode) {
  SetLastError(errorcode);
  ImplWrapSErrSetLastError(errorcode);
}

// @468
void STORMAPI SErrSuppressErrors(BOOL suppress) {
  ImplWrapSErrSuppressErrors(suppress);
}

// @562
BOOL __cdecl SErrDisplayErrorFmt(DWORD errorcode, LPCTSTR filename, int linenumber, BOOL recoverable, DWORD exitcode, LPCTSTR format, ...) {
  char desc[2048];
  
  std::va_list args;
  va_start(args, format);
  vsnprintf(desc, sizeof(desc) - 1, format, args);
  va_end(args);

  return SErrDisplayError(errorcode, filename, linenumber, desc, recoverable, exitcode);
}
