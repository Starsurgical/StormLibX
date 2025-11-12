#ifndef __STORMLIBX_SFILE_H__
#define __STORMLIBX_SFILE_H__

#include "StormTypes.h"


DECLARE_HANDLE(HSFILE);
DECLARE_HANDLE(HSARCHIVE);

#define SFILE_ERRORMODE_RETURNCODE 0
#define SFILE_ERRORMODE_CUSTOM 1
#define SFILE_ERRORMODE_FATAL 2

#define SFILE_OPEN_FROM_MPQ 0
#define SFILE_OPEN_FROM_DISK 1
#define SFILE_OPEN_RELATIVE 2
#define SFILE_OPEN_CHECK_EXISTS 4

typedef BOOL(STORMAPI* SFILEERRORPROC)(LPCSTR, DWORD, DWORD);


extern "C" {

  // @252
  BOOL STORMAPI SFileCloseArchive(HSARCHIVE handle);

  // @253
  BOOL STORMAPI SFileCloseFile(HSFILE handle);

  /*
  // @254
  BOOL STORMAPI SFileDdaBegin(HSFILE handle, DWORD buffersize, DWORD flags);

  // @255
  BOOL STORMAPI SFileDdaBeginEx(HSFILE handle, DWORD buffersize, DWORD flags, DWORD offset, LONG volume, LONG pan = 0, LPVOID reserved = NULL);

  // @256
  BOOL STORMAPI SFileDdaDestroy();

  // @257
  BOOL STORMAPI SFileDdaEnd(HSFILE handle);

  // @258
  BOOL STORMAPI SFileDdaGetPos(HSFILE handle, DWORD* position, DWORD* maxposition);

  // @259
  BOOL STORMAPI SFileDdaGetVolume(HSFILE handle, DWORD*, DWORD*);

  // @260
  BOOL STORMAPI SFileDdaInitialize(void* directsound);

  // @261
  BOOL STORMAPI SFileDdaSetVolume(HSFILE handle, LONG volume, LONG pan = 0);
  */

  // @262
  BOOL STORMAPI SFileDestroy();

  // @263
  BOOL STORMAPI SFileEnableDirectAccess(DWORD flags);

  // @264
  BOOL STORMAPI SFileGetFileArchive(HSFILE file, HSARCHIVE* archive);

  // @265
  DWORD STORMAPI SFileGetFileSize(HSFILE handle, LPDWORD filesizehigh = NULL);

  // @266
  BOOL STORMAPI SFileOpenArchive(LPCSTR archivename, int priority, DWORD flags, HSARCHIVE* handle);

  // @267
  BOOL STORMAPI SFileOpenFile(LPCSTR filename, HSFILE* handle);

  // @268
  BOOL STORMAPI SFileOpenFileEx(HSARCHIVE archivehandle, LPCSTR filename, DWORD flags, HSFILE* handle);

  // @269
  BOOL STORMAPI SFileReadFile(HSFILE handle, LPVOID buffer, DWORD bytestoread, LPDWORD bytesread = NULL, LPOVERLAPPED overlapped = NULL);

  // @270
  //BOOL STORMAPI SFileSetBasePath(LPCSTR path);

  // @271
  DWORD SFileSetFilePointer(HSFILE handle, LONG lDistanceToMove, LONG* lDistanceToMoveHigh, DWORD dwMoveMethod);

  // @272
  BOOL STORMAPI SFileSetLocale(LCID lcid);

  // @273
  //BOOL STORMAPI SFileGetBasePath(LPSTR buffer, DWORD bufferchars);

  // @274
  BOOL STORMAPI SFileSetIoErrorMode(DWORD errormode, SFILEERRORPROC errorproc = NULL);

  // @275
  BOOL STORMAPI SFileGetArchiveName(HSARCHIVE archive, LPSTR buffer, DWORD bufferchars);

  // @276
  BOOL STORMAPI SFileGetFileName(HSFILE file, LPSTR buffer, DWORD bufferchars);

  // @277
  BOOL STORMAPI SFileGetArchiveInfo(HSARCHIVE archive, int* priority, BOOL* cdrom);

  // @278
  void SFileSetPlatform(BYTE platform);

  // @279
  BOOL STORMAPI SFileLoadFile(LPCSTR filename, LPVOID* buffer, DWORD* buffersize, DWORD extrasize, LPOVERLAPPED lpOverlapped);

  // @280
  BOOL STORMAPI SFileUnloadFile(LPVOID file);

  // @281
  BOOL STORMAPI SFileLoadFileEx(HSARCHIVE hArchive, LPCSTR filename, LPVOID* buffer, LPDWORD buffersize, DWORD extrasize, DWORD searchScope, struct _OVERLAPPED* lpOverlapped);

  // @282
  //void STORMAPI SFilePrioritizeRequest(int,int);

  // @283
  //void STORMAPI SFileCancelRequest(int);

  // @284
  void STORMAPI SFileSetAsyncBudget(DWORD budget);

  // @285
  void STORMAPI SFileSetDataChunkSize(DWORD chunksize);

  // @286
  //BOOL STORMAPI SFileEnableSeekOptimization();

  // @287
  //BOOL STORMAPI SFileReadFileEx();

  // @288
  //BOOL STORMAPI SFileFileExists();

  // @289
  //BOOL STORMAPI SFileFileExistsEx();

  // @290
  //BOOL STORMAPI SFileReadFileEx2();

  // @291
  //BOOL STORMAPI SFileReadFile2();

  // @292
  //BOOL STORMAPI SFileLoadFile2();

  // @293
  //BOOL STORMAPI SFileOpenFileAsArchive();

  // @294
  LCID STORMAPI SFileGetLocale();

  // @295
  void STORMAPI SFileGetInstallPath(LPSTR dest, DWORD destsize, BOOL includeseparator = FALSE);

  // @296
  void STORMAPI SFileGetUserDataPath(LPSTR dest, DWORD destsize, BOOL includeseparator);

  // @297
  void STORMAPI SFileSetUserDataPath(LPCSTR directory);
}

#endif
