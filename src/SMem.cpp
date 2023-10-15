#include "SMem.h"

#include <cstring>
#include <storm/Memory.hpp>

// @401
LPVOID STORMAPI SMemAlloc(DWORD bytes, LPCSTR filename, int linenumber, DWORD flags) {
  return ImplWrapSMemAlloc(bytes, filename, linenumber, flags);
}

// @403
BOOL STORMAPI SMemFree(LPVOID ptr, LPCSTR filename, int linenumber, DWORD flags) {
  ImplWrapSMemFree(ptr, filename, linenumber, flags);
  return TRUE;
}

// @405
LPVOID STORMAPI SMemReAlloc(LPVOID ptr, DWORD bytes, LPCSTR filename, int linenumber, DWORD flags) {
  return ImplWrapSMemReAlloc(ptr, bytes, filename, linenumber, flags);
}

// @491
void STORMAPI SMemCopy(void* dest, const void* source, size_t size) {
  ImplWrapSMemCopy(dest, source, size);
}

// @492
void STORMAPI SMemFill(void* location, size_t length, BYTE fillWith) {
  ImplWrapSMemFill(location, length, fillWith);
}

// @493
void STORMAPI SMemMove(void* dest, const void* source, size_t size) {
  ImplWrapSMemMove(dest, source, size);
}

// @494
void STORMAPI SMemZero(void* location, size_t length) {
  ImplWrapSMemZero(location, length);
}

// @495
int STORMAPI SMemCmp(void* location1, void* location2, DWORD size) {
  return std::memcmp(location1, location2, size);
}

