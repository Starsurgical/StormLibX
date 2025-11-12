/*****************************************************************************/
/* DllMain.c                              Copyright (c) Ladislav Zezula 2006 */
/*---------------------------------------------------------------------------*/
/* Description: DllMain for the StormLib.dll library                         */
/*---------------------------------------------------------------------------*/
/*   Date    Ver   Who  Comment                                              */
/* --------  ----  ---  -------                                              */
/* 23.11.06  1.00  Lad  The first version of DllMain.c                       */
/*****************************************************************************/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

extern "C" {
  void StormInitialize();
  void StormDestroyWrapped();
}

extern HINSTANCE s_instance;

//-----------------------------------------------------------------------------
// DllMain

BOOL WINAPI DllMain(HINSTANCE hInst, DWORD dwReason, LPVOID lpReserved)
{
  UNREFERENCED_PARAMETER(lpReserved);
  UNREFERENCED_PARAMETER(hInst);
  if (dwReason == DLL_PROCESS_ATTACH) {
    s_instance = hInst;
    StormInitialize();
  }
  else if (dwReason == DLL_PROCESS_DETACH) {
    StormDestroyWrapped();
  }
  return TRUE;
}
