#include "SErr.h"
#include "SStr.h"

#include <storm/Error.hpp>
#include <StormLib.h>
#include <cstdarg>

void* s_err_callback;


const char* GetStormErrorString(DWORD errorcode) {
  switch (errorcode) {
  case 0x85100000: return "ASSERTION!\r\n";
  case 0x85100065: return "STORM_ERROR_BAD_ARGUMENT\r\n";
  case 0x85100066: return "STORM_ERROR_GAME_ALREADY_STARTED\r\n";
  case 0x85100067: return "STORM_ERROR_GAME_FULL\r\n";
  case 0x85100068: return "STORM_ERROR_GAME_NOT_FOUND\r\n";
  case 0x85100069: return "STORM_ERROR_GAME_TERMINATED\r\n";
  case 0x8510006A: return "STORM_ERROR_INVALID_PLAYER\r\n";
  case 0x8510006B: return "STORM_ERROR_NO_MESSAGES_WAITING\r\n";
  case 0x8510006C: return "STORM_ERROR_NOT_ARCHIVE\r\n";
  case 0x8510006D: return "STORM_ERROR_NOT_ENOUGH_ARGUMENTS\r\n";
  case 0x8510006E: return "STORM_ERROR_NOT_IMPLEMENTED\r\n";
  case 0x8510006F: return "STORM_ERROR_NOT_IN_ARCHIVE\r\n";
  case 0x85100070: return "STORM_ERROR_NOT_IN_GAME\r\n";
  case 0x85100071: return "STORM_ERROR_NOT_INITIALIZED\r\n";
  case 0x85100072: return "STORM_ERROR_NOT_PLAYING\r\n";
  case 0x85100073: return "STORM_ERROR_NOT_REGISTERED\r\n";
  case 0x85100074: return "STORM_ERROR_REQUIRES_CODEC\r\n";
  case 0x85100075: return "STORM_ERROR_REQUIRES_CODEC\r\n";
  case 0x85100076: return "STORM_ERROR_REQUIRES_CODEC\r\n";
  case 0x85100077: return "STORM_ERROR_REQUIRES_UPGRADE\r\n";
  case 0x85100078: return "STORM_ERROR_STILL_ACTIVE\r\n";
  case 0x85100079: return "STORM_ERROR_VERSION_MISMATCH\r\n";
  case 0x8510007A: return "Attempt to free a memory block which is not currently allocated.\r\n";
  case 0x8510007B: return "This memory block has been corrupted by an out-of-bounds memory write.\r\n";
  case 0x8510007C: return "Attempt to free an invalid memory block.\r\n";
  case 0x8510007D: return "This function call is invalid because the memory manager is not currently initialized.\r\n";
  case 0x8510007E: return "A block of memory was allocated but never freed.\r\n";
  case 0x8510007F: return "A resource handle was obtained but never released.\r\n";
  case 0x85100080: return "Attempt to access beyond the bounds of an array.\r\n";
  case 0x85100081: return "Attempt to free a NULL pointer.\r\n";
  case 0x85100082: return "The CD keys do not match.\r\n";
  case 0x85100083: return "The file data is corrupt.\r\n";
  case 0x85100084: return "FATAL ERROR!\r\n";
  case 0x85100085: return "The requested game type is temporarily unavailable.\r\n";
  case 0x88760005: return "DDERR_ALREADYINITIALIZED\r\n";
  case 0x8876000A: return "DDERR_CANNOTATTACHSURFACE\r\n";
  case 0x88760014: return "DDERR_CANNOTDETACHSURFACE\r\n";
  case 0x88760028: return "DDERR_CURRENTLYNOTAVAIL\r\n";
  case 0x88760037: return "DDERR_EXCEPTION\r\n";
  case 0x8876005A: return "DDERR_HEIGHTALIGN\r\n";
  case 0x8876005F: return "DDERR_INCOMPATIBLEPRIMARY\r\n";
  case 0x88760064: return "DDERR_INVALIDCAPS\r\n";
  case 0x8876006E: return "DDERR_INVALIDCLIPLIST\r\n";
  case 0x88760078: return "DDERR_INVALIDMODE\r\n";
  case 0x88760082: return "DDERR_INVALIDOBJECT\r\n";
  case 0x88760091: return "DDERR_INVALIDPIXELFORMAT\r\n";
  case 0x88760096: return "DDERR_INVALIDRECT\r\n";
  case 0x887600A0: return "DDERR_LOCKEDSURFACES\r\n";
  case 0x887600AA: return "DDERR_NO3D\r\n";
  case 0x887600B4: return "DDERR_NOALPHAHW\r\n";
  case 0x887600CD: return "DDERR_NOCLIPLIST\r\n";
  case 0x887600D2: return "DDERR_NOCOLORCONVHW\r\n";
  case 0x887600D4: return "DDERR_NOCOOPERATIVELEVELSET\r\n";
  case 0x887600D7: return "DDERR_NOCOLORKEY\r\n";
  case 0x887600DC: return "DDERR_NOCOLORKEYHW\r\n";
  case 0x887600DE: return "DDERR_NODIRECTDRAWSUPPORT\r\n";
  case 0x887600E1: return "DDERR_NOEXCLUSIVEMODE\r\n";
  case 0x887600E6: return "DDERR_NOFLIPHW\r\n";
  case 0x887600F0: return "DDERR_NOGDI\r\n";
  case 0x887600FA: return "DDERR_NOMIRRORHW\r\n";
  case 0x887600FF: return "DDERR_NOTFOUND\r\n";
  case 0x88760104: return "DDERR_NOOVERLAYHW\r\n";
  case 0x88760118: return "DDERR_NORASTEROPHW\r\n";
  case 0x88760122: return "DDERR_NOROTATIONHW\r\n";
  case 0x88760136: return "DDERR_NOSTRETCHHW\r\n";
  case 0x8876013C: return "DDERR_NOT4BITCOLOR\r\n";
  case 0x8876013D: return "DDERR_NOT4BITCOLORINDEX\r\n";
  case 0x88760140: return "DDERR_NOT8BITCOLOR\r\n";
  case 0x8876014A: return "DDERR_NOTEXTUREHW\r\n";
  case 0x8876014F: return "DDERR_NOVSYNCHW\r\n";
  case 0x88760154: return "DDERR_NOZBUFFERHW\r\n";
  case 0x8876015E: return "DDERR_NOZOVERLAYHW\r\n";
  case 0x88760168: return "DDERR_OUTOFCAPS\r\n";
  case 0x8876017C: return "DDERR_OUTOFVIDEOMEMORY\r\n";
  case 0x8876017E: return "DDERR_OVERLAYCANTCLIP\r\n";
  case 0x88760180: return "DDERR_OVERLAYCOLORKEYONLYONEACTIVE\r\n";
  case 0x88760183: return "DDERR_PALETTEBUSY\r\n";
  case 0x88760190: return "DDERR_COLORKEYNOTSET\r\n";
  case 0x8876019A: return "DDERR_SURFACEALREADYATTACHED\r\n";
  case 0x887601A4: return "DDERR_SURFACEALREADYDEPENDENT\r\n";
  case 0x887601AE: return "DDERR_SURFACEBUSY\r\n";
  case 0x887601B3: return "DDERR_CANTLOCKSURFACE\r\n";
  case 0x887601B8: return "DDERR_SURFACEISOBSCURED\r\n";
  case 0x887601C2: return "DDERR_SURFACELOST\r\n";
  case 0x887601CC: return "DDERR_SURFACENOTATTACHED\r\n";
  case 0x887601D6: return "DDERR_TOOBIGHEIGHT\r\n";
  case 0x887601E0: return "DDERR_TOOBIGSIZE\r\n";
  case 0x887601EA: return "DDERR_TOOBIGWIDTH\r\n";
  case 0x887601FE: return "DDERR_UNSUPPORTEDFORMAT\r\n";
  case 0x88760208: return "DDERR_UNSUPPORTEDMASK\r\n";
  case 0x88760219: return "DDERR_VERTICALBLANKINPROGRESS\r\n";
  case 0x8876021C: return "DDERR_WASSTILLDRAWING\r\n";
  case 0x88760230: return "DDERR_XALIGN\r\n";
  case 0x88760231: return "DDERR_INVALIDDIRECTDRAWGUID\r\n";
  case 0x88760232: return "DDERR_DIRECTDRAWALREADYCREATED\r\n";
  case 0x88760233: return "DDERR_NODIRECTDRAWHW\r\n";
  case 0x88760234: return "DDERR_PRIMARYSURFACEALREADYEXISTS\r\n";
  case 0x88760235: return "DDERR_NOEMULATION\r\n";
  case 0x88760236: return "DDERR_REGIONTOOSMALL\r\n";
  case 0x88760237: return "DDERR_CLIPPERISUSINGHWND\r\n";
  case 0x88760238: return "DDERR_NOCLIPPERATTACHED\r\n";
  case 0x88760239: return "DDERR_NOHWND\r\n";
  case 0x8876023A: return "DDERR_HWNDSUBCLASSED\r\n";
  case 0x8876023B: return "DDERR_HWNDALREADYSET\r\n";
  case 0x8876023C: return "DDERR_NOPALETTEATTACHED\r\n";
  case 0x8876023D: return "DDERR_NOPALETTEHW\r\n";
  case 0x8876023E: return "DDERR_BLTFASTCANTCLIP\r\n";
  case 0x8876023F: return "DDERR_NOBLTHW\r\n";
  case 0x88760240: return "DDERR_NODDROPSHW\r\n";
  case 0x88760241: return "DDERR_OVERLAYNOTVISIBLE\r\n";
  case 0x88760242: return "DDERR_NOOVERLAYDEST\r\n";
  case 0x88760243: return "DDERR_INVALIDPOSITION\r\n";
  case 0x88760244: return "DDERR_NOTAOVERLAYSURFACE\r\n";
  case 0x88760245: return "DDERR_EXCLUSIVEMODEALREADYSET\r\n";
  case 0x88760246: return "DDERR_NOTFLIPPABLE\r\n";
  case 0x88760247: return "DDERR_CANTDUPLICATE\r\n";
  case 0x88760248: return "DDERR_NOTLOCKED\r\n";
  case 0x88760249: return "DDERR_CANTCREATEDC\r\n";
  case 0x8876024A: return "DDERR_NODC\r\n";
  case 0x8876024B: return "DDERR_WRONGMODE\r\n";
  case 0x8876024C: return "DDERR_IMPLICITLYCREATED\r\n";
  case 0x8876024D: return "DDERR_NOTPALETTIZED\r\n";
  case 0x8876024E: return "DDERR_UNSUPPORTEDMODE\r\n";
  case 0x8876024F: return "DDERR_NOMIPMAPHW\r\n";
  case 0x88760250: return "DDERR_INVALIDSURFACETYPE\r\n";
  case 0x8876026C: return "DDERR_DCALREADYCREATED\r\n";
  case 0x88760280: return "DDERR_CANTPAGELOCK\r\n";
  case 0x88760294: return "DDERR_CANTPAGEUNLOCK\r\n";
  case 0x887602A8: return "DDERR_NOTPAGELOCKED\r\n";
  case 0x8878000A: return "DSERR_ALLOCATED\r\n";
  case 0x8878001E: return "DSERR_CONTROLUNAVAIL\r\n";
  case 0x88780032: return "DSERR_INVALIDCALL\r\n";
  case 0x88780046: return "DSERR_PRIOLEVELNEEDED\r\n";
  case 0x88780064: return "DSERR_BADFORMAT\r\n";
  case 0x88780078: return "DSERR_NODRIVER\r\n";
  case 0x88780082: return "DSERR_ALREADYINITIALIZED\r\n";
  case 0x88780096: return "DSERR_BUFFERLOST\r\n";
  case 0x887800A0: return "DSERR_OTHERAPPHASPRIO\r\n";
  case 0x887800AA: return "DSERR_UNINITIALIZED\r\n";
  }
  return nullptr;
}

void SErrSetBlizzardErrorFunction(void* pFn) {
  s_err_callback = pFn;
}

// @461
BOOL STORMAPI SErrDisplayError(DWORD errorcode, LPCTSTR filename, int linenumber, LPCTSTR description, BOOL recoverable, DWORD exitcode) {
  return ImplWrapSErrDisplayError(errorcode, filename, linenumber, description, recoverable, exitcode, 0);
}

// @462
BOOL STORMAPI SErrGetErrorStr(DWORD errorcode, LPTSTR buffer, DWORD bufferchars) {
  const char* pszErrorString = GetStormErrorString(errorcode);
  if (pszErrorString) {
    SStrPrintf(buffer, bufferchars, "%s", pszErrorString);
    return TRUE;
  }
  SStrPrintf(buffer, bufferchars, "ERROR: 0x%08X\r\n", errorcode);
  return TRUE;
}

// @463
DWORD STORMAPI SErrGetLastError() {
  DWORD err = GetLastError();
  if (err == ERROR_SUCCESS) {
    err = ImplWrapSErrGetLastError();
  }
  return err;
}

// @465
void STORMAPI SErrSetLastError(DWORD errorcode) {
  SetLastError(errorcode);
  ImplWrapSErrSetLastError(errorcode);
}

// @468
void STORMAPI SErrSuppressErrors(BOOL suppress) {
  ImplWrapSErrSuppressErrors(suppress);
}

// @562
BOOL __cdecl SErrDisplayErrorFmt(DWORD errorcode, LPCTSTR filename, int linenumber, BOOL recoverable, DWORD exitcode, LPCTSTR format, ...) {
  char desc[2048];
  
  std::va_list args;
  va_start(args, format);
  vsnprintf(desc, sizeof(desc) - 1, format, args);
  va_end(args);

  return SErrDisplayError(errorcode, filename, linenumber, desc, recoverable, exitcode);
}
