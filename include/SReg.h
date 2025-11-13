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
  BOOL STORMAPI SRegLoadData(const char* keyname, const char* valuename, std::uint32_t flags, void* buffer, std::uint32_t buffersize, std::uint32_t* bytesread);

  // @422
  BOOL STORMAPI SRegLoadString(const char* keyname, const char* valuename, std::uint32_t flags, char* buffer, std::uint32_t bufferchars);

  // @423
  BOOL STORMAPI SRegLoadValue(const char* keyname, const char* valuename, std::uint32_t flags, std::uint32_t* value);

  // @424
  BOOL STORMAPI SRegSaveData(const char* keyname, const char* valuename, std::uint32_t flags, void* data, std::uint32_t databytes);

  // @425
  BOOL STORMAPI SRegSaveString(const char* keyname, const char* valuename, std::uint32_t flags, const char* string);

  // @426
  BOOL STORMAPI SRegSaveValue(const char* keyname, const char* valuename, std::uint32_t flags, std::uint32_t value);

  // @427
  BOOL STORMAPI SRegGetBaseKey(std::uint32_t flags, char* buffer, std::uint32_t buffersize);

  // @428
  BOOL STORMAPI SRegDeleteValue(const char* keyname, const char* valuename, std::uint32_t flags);

  // @429
  BOOL STORMAPI SRegEnumKey(const char* keyname, std::uint32_t flags, std::uint32_t index, char* buffer, std::uint32_t bufferchars);

  // @430
  BOOL STORMAPI SRegGetNumSubKeys(const char* keyname, std::uint32_t flags, std::uint32_t* subkeys);
}

#endif
