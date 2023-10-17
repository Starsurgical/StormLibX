#ifndef __STORMLIBX_SDRAW_H__
#define __STORMLIBX_SDRAW_H__

#include "StormTypes.h"

extern "C" {
  // @343
  BOOL STORMAPI SDrawCaptureScreen(LPCTSTR filename = NULL, BOOL usesavepath = FALSE);

  // @344
  BOOL STORMAPI SDrawClearSurface(int surfacenumber);

  // @345
  void STORMAPI SDrawDestroy();

  // @346
  HWND STORMAPI SDrawGetFrameWindow(HWND* window = NULL);

  // @348
  BOOL STORMAPI SDrawGetScreenSize(int* width, int* height, int* bitdepth);

  // @350
  BOOL STORMAPI SDrawLockSurface(int surfacenumber, LPCRECT rect, LPBYTE* ptr, int* pitch = NULL, DWORD flags = 0);

  // @351
  //BOOL STORMAPI SDrawManualInitialize(HWND framewindow, LPDIRECTDRAW directdraw, LPDIRECTDRAWSURFACE frontbuffer, LPDIRECTDRAWSURFACE backbuffer, LPDIRECTDRAWSURFACE systembuffer, LPDIRECTDRAWSURFACE temporarybuffer);
  BOOL STORMAPI SDrawManualInitialize(HWND framewindow, void* directdraw, void* frontbuffer, void* backbuffer, void* systembuffer, void* temporarybuffer);

  // @352
  int STORMAPI SDrawMessageBox(LPCTSTR text, LPCTSTR title, DWORD flags);

  // @356
  BOOL STORMAPI SDrawUnlockSurface(int surfacenumber, LPBYTE ptr, DWORD numrects = 0, LPCRECT rectarray = NULL);

  // @357
  BOOL STORMAPI SDrawUpdatePalette(DWORD firstentry, DWORD numentries, LPPALETTEENTRY entries, BOOL reservedentries = FALSE);
}

#endif
