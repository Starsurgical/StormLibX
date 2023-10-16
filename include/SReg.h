#ifndef __STORMLIBX_SREG_H__
#define __STORMLIBX_SREG_H__

#include "StormTypes.h"


#define SREG_FLAG_USERSPECIFIC  0x00000001
#define SREG_FLAG_BATTLENET     0x00000002
#define SREG_FLAG_FLUSHTODISK   0x00000008
#define SREG_FLAG_OVERRIDEPATH  0x00000010
#define SREG_FLAG_MULTISZ       0x00000080


// @421
BOOL STORMAPI SRegLoadData(LPCTSTR keyname, LPCTSTR valuename, DWORD flags, LPVOID buffer, DWORD buffersize, DWORD* bytesread);

// @422
BOOL STORMAPI SRegLoadString(LPCTSTR keyname, LPCTSTR valuename, DWORD flags, LPTSTR buffer, DWORD bufferchars);

// @423
BOOL STORMAPI SRegLoadValue(LPCTSTR keyname, LPCTSTR valuename, DWORD flags, DWORD* value);

// @424
BOOL STORMAPI SRegSaveData(LPCTSTR keyname, LPCTSTR valuename, DWORD flags, LPVOID data, DWORD databytes);

// @425
BOOL STORMAPI SRegSaveString(LPCTSTR keyname, LPCTSTR valuename, DWORD flags, LPCTSTR string);

// @426
BOOL STORMAPI SRegSaveValue(LPCTSTR keyname, LPCTSTR valuename, DWORD flags, DWORD value);

// @427
BOOL STORMAPI SRegGetBaseKey(DWORD flags, LPTSTR buffer, DWORD buffersize);

// @428
BOOL STORMAPI SRegDeleteValue(LPCTSTR keyname, LPCTSTR valuename, DWORD flags);

// @429
BOOL STORMAPI SRegEnumKey(LPCTSTR keyname, DWORD flags, DWORD index, LPTSTR buffer, DWORD bufferchars);

// @430
BOOL STORMAPI SRegGetNumSubKeys(LPCTSTR keyname, DWORD flags, DWORD* subkeys);

#endif
