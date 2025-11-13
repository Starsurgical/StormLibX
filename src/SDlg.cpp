#include "SDlg.h"
#include <SDL_mouse.h>
#include <SDL_timer.h>


int s_throttleDialogMessageLoop;


BOOL STORMAPI SDlgSetCursor(HWND window, HCURSOR cursor, std::uint32_t id, HCURSOR *oldcursor) {
  if (oldcursor) *oldcursor = reinterpret_cast<HCURSOR>(SDL_GetCursor());
  SDL_SetCursor(reinterpret_cast<SDL_Cursor*>(cursor));
  return TRUE;
}

int STORMAPI SDlgSetMessageLoopThrottle(int throttle) {
  s_throttleDialogMessageLoop = throttle;
  return throttle;
}
