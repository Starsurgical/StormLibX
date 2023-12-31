#ifndef __STORMLIBX_SERR_H__
#define __STORMLIBX_SERR_H__

#include "StormTypes.h"

#ifndef ERROR_GEN_FAILURE
#define ERROR_GEN_FAILURE 31
#endif

#ifndef ERROR_INVALID_PARAMETER
#define ERROR_INVALID_PARAMETER 87
#endif

#ifndef ERROR_BAD_PROVIDER
#define ERROR_BAD_PROVIDER 1204
#endif


#define STORM_ERROR_ASSERTION                0x85100000
#define STORM_ERROR_BAD_ARGUMENT             0x85100065
#define STORM_ERROR_GAME_ALREADY_STARTED     0x85100066
#define STORM_ERROR_GAME_FULL                0x85100067
#define STORM_ERROR_GAME_NOT_FOUND           0x85100068
#define STORM_ERROR_GAME_TERMINATED          0x85100069
#define STORM_ERROR_INVALID_PLAYER           0x8510006A
#define STORM_ERROR_NO_MESSAGES_WAITING      0x8510006B
#define STORM_ERROR_NOT_ARCHIVE              0x8510006C
#define STORM_ERROR_NOT_ENOUGH_ARGUMENTS     0x8510006D
#define STORM_ERROR_NOT_IMPLEMENTED          0x8510006E
#define STORM_ERROR_NOT_IN_ARCHIVE           0x8510006F
#define STORM_ERROR_NOT_IN_GAME              0x85100070
#define STORM_ERROR_NOT_INITIALIZED          0x85100071
#define STORM_ERROR_NOT_PLAYING              0x85100072
#define STORM_ERROR_NOT_REGISTERED           0x85100073
#define STORM_ERROR_REQUIRES_CODEC           0x85100074
#define STORM_ERROR_REQUIRES_DDRAW           0x85100075
#define STORM_ERROR_REQUIRES_DSOUND          0x85100076
#define STORM_ERROR_REQUIRES_UPGRADE         0x85100077
#define STORM_ERROR_STILL_ACTIVE             0x85100078
#define STORM_ERROR_VERSION_MISMATCH         0x85100079
#define STORM_ERROR_MEMORY_ALREADY_FREED     0x8510007A
#define STORM_ERROR_MEMORY_CORRUPT           0x8510007B
#define STORM_ERROR_MEMORY_INVALID_BLOCK     0x8510007C
#define STORM_ERROR_MEMORY_MANAGER_INACTIVE  0x8510007D
#define STORM_ERROR_MEMORY_NEVER_RELEASED    0x8510007E
#define STORM_ERROR_HANDLE_NEVER_RELEASED    0x8510007F
#define STORM_ERROR_ACCESS_OUT_OF_BOUNDS     0x85100080
#define STORM_ERROR_MEMORY_NULL_POINTER      0x85100081
// guessed
#define STORM_ERROR_CDKEY_MISMATCH            0x85100082
#define STORM_ERROR_DATA_FILE_CORRUPT         0x85100083
#define STORM_ERROR_FATAL                     0x85100084
#define STORM_ERROR_GAME_TYPE_UNAVAILABLE     0x85100085


#define SERR_LINECODE_FUNCTION  -1
#define SERR_LINECODE_OBJECT    -2
#define SERR_LINECODE_HANDLE    -3
#define SERR_LINECODE_FILE      -4


#define VALIDATEBEGIN do { int __validate_result = -1
#define VALIDATE(Condition) __validate_result &= (Condition) ? -1 : 0
#define VALIDATEANDBLANK(Var) if (Var) *Var = 0; else __validate_result = 0
#define VALIDATEEND if (!__validate_result) { SErrSetLastError(ERROR_INVALID_PARAMETER); return 0; } \
                    }while(0)


typedef void(__fastcall* RECEIVEERRORPROC)(LPCSTR, LPCSTR, LPCSTR*, DWORD, DWORD, bool);

extern "C" {

  void SErrSetBlizzardErrorFunction(RECEIVEERRORPROC pFn);

  // @461
  BOOL STORMAPI SErrDisplayError(DWORD errorcode, LPCTSTR filename, int linenumber, LPCTSTR description, BOOL recoverable, DWORD exitcode);

  // @462
  BOOL STORMAPI SErrGetErrorStr(DWORD errorcode, LPTSTR buffer, DWORD bufferchars);

  // @463
  DWORD STORMAPI SErrGetLastError();

  // @465
  void STORMAPI SErrSetLastError(DWORD errorcode);

  // @468
  void STORMAPI SErrSuppressErrors(BOOL suppress);

  // @562
  BOOL __cdecl SErrDisplayErrorFmt(DWORD errorcode, LPCTSTR filename, int linenumber, BOOL recoverable, DWORD exitcode, LPCTSTR format, ...);
}

#endif
