#ifndef __STORMLIBX_SRGN_H__
#define __STORMLIBX_SRGN_H__

#include "StormTypes.h"


// @521
void STORMAPI SRgnClear(HSRGN handle);

// @523
void STORMAPI SRgnCombineRect(HSRGN handle, LPRECT rect, LPVOID param, int combinemode);

// @524
void STORMAPI SRgnCreate(HSRGN* handle, DWORD reserved = 0);

// @525
void STORMAPI SRgnDelete(HSRGN handle);

// @527
//void STORMAPI SRgnDuplicate(HSRGN orighandle, HSRGN handle, int reserved = 0);

// @528
//void STORMAPI SRgnGetRectParams(HSRGN handle, LPCRECT rect, DWORD* numparams, DWORD* buffer);

// @529
//void STORMAPI SRgnGetRects(HSRGN handle, DWORD* numrects, LPRECT buffer);

// @530
void STORMAPI SRgnGetBoundingRect(HSRGN handle, LPRECT rect);

// @531
BOOL STORMAPI SRgnIsPointInRegion(HSRGN handle, int x, int y);

// @532
BOOL STORMAPI SRgnIsRectInRegion(HSRGN handle, LPRECT rect);

// @533
//void STORMAPI SRgnOffset(HSRGN handle, int xoffset, int yoffset);

// @534
void STORMAPI SRgnCombineRectf(HSRGN handle, RECTF* rect, LPVOID param, int combinemode);

// @535
//void STORMAPI SRgnGetRectParams(HSRGN handle, RECTF* rect, DWORD* numparams, DWORD* buffer);

// @536
//void STORMAPI SRgnGetRectsf(HSRGN handle, DWORD* numrects, RECTF* buffer);

// @537
void STORMAPI SRgnGetBoundingRectf(HSRGN handle, RECTF* rect);

// @538
BOOL STORMAPI SRgnIsPointInRegionf(HSRGN handle, float x, float y);

// @539
BOOL STORMAPI SRgnIsRectInRegionf(HSRGN handle, RECTF *rect);

// @540
//void STORMAPI SRgnOffsetf(HSRGN handle, float xoffset, float yoffset);



#endif
