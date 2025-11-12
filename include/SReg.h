#ifndef __STORMLIBX_SREG_H__
#define __STORMLIBX_SREG_H__

#include "StormTypes.h"


#define SREG_FLAG_USERSPECIFIC  0x00000001
#define SREG_FLAG_BATTLENET     0x00000002
#define SREG_FLAG_FLUSHTODISK   0x00000008
#define SREG_FLAG_OVERRIDEPATH  0x00000010
#define SREG_FLAG_MULTISZ       0x00000080

extern "C" {

  // @421
  BOOL STORMAPI SRegLoadData(LPCSTR keyname, LPCSTR valuename, DWORD flags, LPVOID buffer, DWORD buffersize, DWORD* bytesread);

  // @422
  BOOL STORMAPI SRegLoadString(LPCSTR keyname, LPCSTR valuename, DWORD flags, LPSTR buffer, DWORD bufferchars);

  // @423
  BOOL STORMAPI SRegLoadValue(LPCSTR keyname, LPCSTR valuename, DWORD flags, DWORD* value);

  // @424
  BOOL STORMAPI SRegSaveData(LPCSTR keyname, LPCSTR valuename, DWORD flags, LPVOID data, DWORD databytes);

  // @425
  BOOL STORMAPI SRegSaveString(LPCSTR keyname, LPCSTR valuename, DWORD flags, LPCSTR string);

  // @426
  BOOL STORMAPI SRegSaveValue(LPCSTR keyname, LPCSTR valuename, DWORD flags, DWORD value);

  // @427
  BOOL STORMAPI SRegGetBaseKey(DWORD flags, LPSTR buffer, DWORD buffersize);

  // @428
  BOOL STORMAPI SRegDeleteValue(LPCSTR keyname, LPCSTR valuename, DWORD flags);

  // @429
  BOOL STORMAPI SRegEnumKey(LPCSTR keyname, DWORD flags, DWORD index, LPSTR buffer, DWORD bufferchars);

  // @430
  BOOL STORMAPI SRegGetNumSubKeys(LPCSTR keyname, DWORD flags, DWORD* subkeys);
}

#endif
