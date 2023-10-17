#ifndef __STORMLIBX_STORMTYPES_H__
#define __STORMLIBX_STORMTYPES_H__

#include <StormPort.h>
#include <storm/Array.hpp>
#include <storm/Handle.hpp>
#include <storm/Hash.hpp>
#include <storm/List.hpp>
#include <storm/Queue.hpp>
#include <storm/thread/CCritSect.hpp>
#include <storm/region/Types.hpp>

typedef int BOOL;
#define FALSE 0
#define TRUE 1

#ifndef STORMAPI
#define STORMAPI __stdcall
#endif

#ifndef MB_OK
#define MB_OK                   0x00000000L
#define MB_OKCANCEL             0x00000001L
#define MB_ABORTRETRYIGNORE     0x00000002L
#define MB_YESNOCANCEL          0x00000003L
#define MB_YESNO                0x00000004L
#define MB_RETRYCANCEL          0x00000005L
#define MB_ICONERROR            0x00000010L
#define MB_ICONQUESTION         0x00000020L
#define MB_ICONWARNING          0x00000030L
#define MB_ICONINFORMATION      0x00000040L

#define MB_DEFBUTTON1           0x00000000L
#define MB_DEFBUTTON2           0x00000100L
#define MB_DEFBUTTON3           0x00000200L

#define MB_APPLMODAL            0x00000000L
#define MB_SYSTEMMODAL          0x00001000L
#define MB_TASKMODAL            0x00002000L
#define MB_HELP                 0x00004000L
#define MB_NOFOCUS              0x00008000L
#define MB_SETFOREGROUND        0x00010000L
#define MB_DEFAULT_DESKTOP_ONLY 0x00020000L
#define MB_TOPMOST              0x00040000L
#define MB_RIGHT                0x00080000L
#define MB_RTLREADING           0x00100000L

#define MB_TYPEMASK             0x0000000FL
#define MB_ICONMASK             0x000000F0L
#define MB_DEFMASK              0x00000F00L
#define MB_MODEMASK             0x00003000L
#define MB_MISCMASK             0x0000C000L
#endif


#define VALIDATEBEGIN do { int __validate_result = -1
#define VALIDATE(Condition) __validate_result &= (Condition) ? -1 : 0
#define VALIDATEANDBLANK(Var) if (Var) *Var = 0; else __validate_result = 0
#define VALIDATEEND if (!__validate_result) { SErrSetLastError(ERROR_INVALID_PARAMETER); return 0; } \
                    }while(0)


#endif
