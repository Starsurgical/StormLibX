#include <StormLib.h>

#undef SFILE_OPEN_FROM_MPQ
#undef SFILE_OPEN_CHECK_EXISTS
#undef SFILE_OPEN_BASE_FILE
#undef SFILE_OPEN_ANY_LOCALE
#undef SFILE_OPEN_LOCAL_FILE

#include <map>
#include <SDL.h>

#include "SFile.h"
#include "SMem.h"
#include <storm/String.hpp>


#define SLIB_FILE_OPEN_FROM_MPQ     0x00000000
#define SLIB_FILE_OPEN_CHECK_EXISTS 0xFFFFFFFC
#define SLIB_FILE_OPEN_BASE_FILE    0xFFFFFFFD
#define SLIB_FILE_OPEN_ANY_LOCALE   0xFFFFFFFE
#define SLIB_FILE_OPEN_LOCAL_FILE   0xFFFFFFFF


DWORD s_enabledirect;
std::multimap<int, HSARCHIVE> s_archivelist;
DWORD s_ioerrormode;
SFILEERRORPROC s_ioerrorproc;
BYTE s_platform;
DWORD s_asyncbudget;
DWORD s_datachunksize;
char s_savepath[MAX_PATH];

std::map<HSFILE, HSARCHIVE> s_filearchives;

// @252
BOOL STORMAPI SFileCloseArchive(HSARCHIVE handle) {
  for (auto& it = s_filearchives.begin(); it != s_filearchives.end(); ) {
    if (it->second == handle) {
      it = s_filearchives.erase(it);
    }
    else {
      it++;
    }
  }

  for (auto& it = s_archivelist.begin(); it != s_archivelist.end(); ++it) {
    if (it->second == handle) {
      s_archivelist.erase(it);
      break;
    }
  }
  return ImplWrapSFileCloseArchive(handle);
}

// @253
BOOL STORMAPI SFileCloseFile(HSFILE handle) {
  s_filearchives.erase(handle);
  return ImplWrapSFileCloseFile(handle);
}

// @262
BOOL STORMAPI SFileDestroy() {
  BOOL result = TRUE;

  for (auto& [file, archive] : s_filearchives) {
    if (!ImplWrapSFileCloseFile(file)) result = FALSE;
  }
  s_filearchives.clear();

  for (auto& [priority, archive] : s_archivelist) {
    if (!ImplWrapSFileCloseArchive(archive)) result = FALSE;
  }
  s_archivelist.clear();

  return result;
}

// @263
BOOL STORMAPI SFileEnableDirectAccess(DWORD flags) {
  s_enabledirect = flags;
  return TRUE;
}

// @264
BOOL STORMAPI SFileGetFileArchive(HSFILE file, HSARCHIVE* archive) {
  if (archive) *archive = nullptr;

  auto& it = s_filearchives.find(file);
  if (it != s_filearchives.end()) {
    if (archive) *archive = it->second;
    return TRUE;
  }
  return FALSE;
}

// @265
DWORD STORMAPI SFileGetFileSize(HSFILE handle, LPDWORD filesizehigh) {
  return ImplWrapSFileGetFileSize(handle, filesizehigh);
}

// @266
BOOL STORMAPI SFileOpenArchive(LPCSTR archivename, int priority, DWORD flags, HSARCHIVE* handle) {
  flags |= MPQ_OPEN_READ_ONLY | MPQ_OPEN_NO_LISTFILE | MPQ_OPEN_NO_ATTRIBUTES;
  BOOL result = ImplWrapSFileOpenArchive(archivename, priority, flags, reinterpret_cast<HANDLE*>(handle));
  if (handle && *handle) {
    s_archivelist.emplace(priority, *handle);
  }
  return result;
}

// @267
BOOL STORMAPI SFileOpenFile(LPCSTR filename, HSFILE* handle) {
  DWORD flags = 0;
  if (s_enabledirect & 1) flags |= SFILE_OPEN_FROM_DISK;
  if (s_enabledirect & 2) flags |= SFILE_OPEN_RELATIVE;

  if (s_archivelist.empty() && s_enabledirect == 0) flags |= SFILE_OPEN_FROM_DISK;

  return SFileOpenFileEx(NULL, filename, flags, handle);
}

// @268
BOOL STORMAPI SFileOpenFileEx(HSARCHIVE archivehandle, LPCSTR filename, DWORD flags, HSFILE* handle) {
  DWORD dwSearchScope = SLIB_FILE_OPEN_FROM_MPQ;
  if (flags & SFILE_OPEN_FROM_DISK) {
    dwSearchScope = SLIB_FILE_OPEN_LOCAL_FILE;  // ?? maybe
  }

  // TODO other flags unsupported
  // TODO s_platform should be a separate thing in StormLib

  if (archivehandle) {
    if (ImplWrapSFileOpenFileEx(archivehandle, filename, dwSearchScope, reinterpret_cast<HANDLE*>(handle))) {
      s_filearchives[*handle] = archivehandle;
      return TRUE;
    }
    return FALSE;
  }
 
  for (auto it = s_archivelist.rbegin(); it != s_archivelist.rend(); ++it) {
    if (ImplWrapSFileOpenFileEx(it->second, filename, dwSearchScope, reinterpret_cast<HANDLE*>(handle))) {
      s_filearchives[*handle] = archivehandle;
      return TRUE;
    }
  }
  return FALSE;
}

// @269
BOOL STORMAPI SFileReadFile(HSFILE handle, LPVOID buffer, DWORD bytestoread, LPDWORD bytesread, LPOVERLAPPED overlapped) {
  return ImplWrapSFileReadFile(handle, buffer, bytestoread, bytesread, overlapped);
}

// @271
DWORD SFileSetFilePointer(HSFILE handle, LONG lDistanceToMove, LONG* lDistanceToMoveHigh, DWORD dwMoveMethod) {
  return ImplWrapSFileSetFilePointer(handle, lDistanceToMove, lDistanceToMoveHigh, dwMoveMethod);
}

// @272
BOOL STORMAPI SFileSetLocale(LCID lcid) {
  return ImplWrapSFileSetLocale(lcid);
}

// @274
BOOL STORMAPI SFileSetIoErrorMode(DWORD errormode, SFILEERRORPROC errorproc) {
  s_ioerrormode = errormode;
  s_ioerrorproc = errorproc;
  return TRUE;
}

// @275
BOOL STORMAPI SFileGetArchiveName(HSARCHIVE archive, LPSTR buffer, DWORD bufferchars) {
  return SFileGetFileInfo(archive, SFileInfoClass::SFileMpqFileName, buffer, bufferchars, NULL);
}

// @276
BOOL STORMAPI SFileGetFileName(HSFILE file, LPSTR buffer, DWORD bufferchars) {
  TFileEntry fileentry;
  if (SFileGetFileInfo(file, SFileInfoClass::SFileInfoFileEntry, &fileentry, sizeof(fileentry), NULL)) {
    SStrCopy(buffer, fileentry.szFileName, bufferchars);
    return TRUE;
  }
  buffer[0] = '\0';
  return FALSE;
}

// @277
BOOL STORMAPI SFileGetArchiveInfo(HSARCHIVE archive, int* priority, BOOL* cdrom) {
  if (priority) *priority = 0;
  if (cdrom) *cdrom = FALSE;

  for (auto& [prio, mpq] : s_archivelist) {
    if (mpq == archive) {
      if (priority) *priority = prio;
      return TRUE;
    }
  }
  return FALSE;
}

// @278
void SFileSetPlatform(BYTE platform) {
  s_platform = platform;
}

// @279
BOOL STORMAPI SFileLoadFile(LPCSTR filename, LPVOID* buffer, DWORD* buffersize, DWORD extrasize, LPOVERLAPPED lpOverlapped) {
  DWORD flags = 0;
  if (s_enabledirect & 1) flags |= SFILE_OPEN_FROM_DISK;
  if (s_enabledirect & 2) flags |= SFILE_OPEN_RELATIVE;

  if (s_archivelist.empty() && s_enabledirect == 0) flags |= SFILE_OPEN_FROM_DISK;

  return SFileLoadFileEx(NULL, filename, buffer, buffersize, extrasize, flags, lpOverlapped);
}

// @280
BOOL STORMAPI SFileUnloadFile(LPVOID file) {
  if (file) {
    FREE(file);
    return TRUE;
  }
  return FALSE;
}

// @281
BOOL STORMAPI SFileLoadFileEx(HSARCHIVE hArchive, LPCSTR filename, LPVOID* buffer, LPDWORD buffersize, DWORD extrasize, DWORD searchScope, struct _OVERLAPPED* lpOverlapped) {

  HSFILE hFile;
  if (SFileOpenFileEx(hArchive, filename, searchScope, &hFile)) {
    // TODO critical section?
    DWORD dwFileSize = SFileGetFileSize(hFile);
    LPBYTE pData = (LPBYTE)ALLOC(dwFileSize + extrasize);

    if (SFileReadFile(hFile, pData, dwFileSize, NULL, lpOverlapped)) {
      if (extrasize > 0) {
        SMemZero(&pData[dwFileSize], extrasize);
      }

      *buffer = pData;
      if (buffersize) *buffersize = dwFileSize;
    }
    else {
      FREEIFUSED(pData);
    }
  }

  if (hFile) SFileCloseFile(hFile);

  return *buffer != NULL;
}

// @284
void STORMAPI SFileSetAsyncBudget(DWORD budget) {
  s_asyncbudget = budget;
}

// @285
void STORMAPI SFileSetDataChunkSize(DWORD chunksize) {
  s_datachunksize = chunksize;
}

// @294
LCID STORMAPI SFileGetLocale() {
  return ImplWrapSFileGetLocale();
}

// @295
void STORMAPI SFileGetInstallPath(LPSTR dest, DWORD destsize, BOOL includeseparator) {
  const char* path = SDL_GetBasePath();
  char tmp[MAX_PATH];
  SStrCopy(tmp, path, sizeof(tmp));

  char* pEnd = SStrChrR(tmp, '\\');
  if (pEnd && !includeseparator) {
    *pEnd = '\0';
  }

  SStrCopy(dest, tmp, destsize);
}

// @296
void STORMAPI SFileGetUserDataPath(LPSTR dest, DWORD destsize, BOOL includeseparator) {
  int len = SStrPrintf(dest, destsize, "%s", s_savepath);
  if (includeseparator) {
    if (dest[len - 1] != '\\') {
      SStrPack(dest, "\\", destsize - len);
    }
  }
  else if (dest[len - 1] == '\\') {
    dest[len - 1] = '\0';
  }
}

// @297
void STORMAPI SFileSetUserDataPath(LPCSTR directory) {
  SStrCopy(s_savepath, directory, MAX_PATH);
}
