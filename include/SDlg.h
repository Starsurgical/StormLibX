#ifndef __STORMLIBX_SDLG_H__
#define __STORMLIBX_SDLG_H__

#include "StormTypes.h"

extern "C" {

  BOOL STORMAPI SDlgSetCursor(HWND window, HCURSOR cursor, std::uint32_t id, HCURSOR *oldcursor);

  int STORMAPI SDlgSetMessageLoopThrottle(int throttle);
}

#endif
