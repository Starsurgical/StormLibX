#include "SRgn.h"

#include <storm/Region.hpp>


// @521
void STORMAPI SRgnClear(HSRGN handle) {
  ImplWrapSRgnClear(handle);
}

// @523
void STORMAPI SRgnCombineRect(HSRGN handle, LPRECT rect, LPVOID param, int combinemode) {
  ImplWrapSRgnCombineRect(handle, reinterpret_cast<RECTI*>(rect), param, combinemode);
}

// @524
void STORMAPI SRgnCreate(HSRGN* handle, DWORD reserved) {
  ImplWrapSRgnCreate(handle, reserved);
}

// @525
void STORMAPI SRgnDelete(HSRGN handle) {
  ImplWrapSRgnDelete(handle);
}

// @530
void STORMAPI SRgnGetBoundingRect(HSRGN handle, LPRECT rect) {
  ImplWrapSRgnGetBoundingRect(handle, reinterpret_cast<RECTI*>(rect));
}

// @531
BOOL STORMAPI SRgnIsPointInRegion(HSRGN handle, int x, int y) {
  return ImplWrapSRgnIsPointInRegion(handle, x, y);
}

// @532
BOOL STORMAPI SRgnIsRectInRegion(HSRGN handle, LPRECT rect) {
  return ImplWrapSRgnIsRectInRegion(handle, reinterpret_cast<RECTI*>(rect));
}

// @534
void STORMAPI SRgnCombineRectf(HSRGN handle, RECTF* rect, LPVOID param, int combinemode) {
  ImplWrapSRgnCombineRectf(handle, rect, param, combinemode);
}

// @537
void STORMAPI SRgnGetBoundingRectf(HSRGN handle, RECTF* rect) {
  ImplWrapSRgnGetBoundingRectf(handle, rect);
}

// @538
BOOL STORMAPI SRgnIsPointInRegionf(HSRGN handle, float x, float y) {
  return ImplWrapSRgnIsPointInRegionf(handle, x, y);
}

// @539
BOOL STORMAPI SRgnIsRectInRegionf(HSRGN handle, RECTF* rect) {
  return ImplWrapSRgnIsRectInRegionf(handle, rect);
}
