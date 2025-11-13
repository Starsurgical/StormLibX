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

typedef BOOL(STORMAPI* SFILEERRORPROC)(const char*, std::uint32_t, std::uint32_t);


extern "C" {

  // @252
  BOOL STORMAPI SFileCloseArchive(HSARCHIVE handle);

  // @253
  BOOL STORMAPI SFileCloseFile(HSFILE handle);

  /*
  // @254
  BOOL STORMAPI SFileDdaBegin(HSFILE handle, std::uint32_t buffersize, std::uint32_t flags);

  // @255
  BOOL STORMAPI SFileDdaBeginEx(HSFILE handle, std::uint32_t buffersize, std::uint32_t flags, std::uint32_t offset, std::int32_t volume, std::int32_t pan = 0, void* reserved = NULL);

  // @256
  BOOL STORMAPI SFileDdaDestroy();

  // @257
  BOOL STORMAPI SFileDdaEnd(HSFILE handle);

  // @258
  BOOL STORMAPI SFileDdaGetPos(HSFILE handle, std::uint32_t* position, std::uint32_t* maxposition);

  // @259
  BOOL STORMAPI SFileDdaGetVolume(HSFILE handle, std::uint32_t*, std::uint32_t*);

  // @260
  BOOL STORMAPI SFileDdaInitialize(void* directsound);

  // @261
  BOOL STORMAPI SFileDdaSetVolume(HSFILE handle, std::int32_t volume, std::int32_t pan = 0);
  */

  // @262
  BOOL STORMAPI SFileDestroy();

  // @263
  BOOL STORMAPI SFileEnableDirectAccess(std::uint32_t flags);

  // @264
  BOOL STORMAPI SFileGetFileArchive(HSFILE file, HSARCHIVE* archive);

  // @265
  std::uint32_t STORMAPI SFileGetFileSize(HSFILE handle, unsigned long* filesizehigh = NULL);

  // @266
  BOOL STORMAPI SFileOpenArchive(const char* archivename, int priority, std::uint32_t flags, HSARCHIVE* handle);

  // @267
  BOOL STORMAPI SFileOpenFile(const char* filename, HSFILE* handle);

  // @268
  BOOL STORMAPI SFileOpenFileEx(HSARCHIVE archivehandle, const char* filename, std::uint32_t flags, HSFILE* handle);

  // @269
  BOOL STORMAPI SFileReadFile(HSFILE handle, void* buffer, std::uint32_t bytestoread, unsigned long* bytesread = NULL, LPOVERLAPPED overlapped = NULL);

  // @270
  //BOOL STORMAPI SFileSetBasePath(const char* path);

  // @271
  std::uint32_t SFileSetFilePointer(HSFILE handle, std::int32_t lDistanceToMove, long* lDistanceToMoveHigh, std::uint32_t dwMoveMethod);

  // @272
  BOOL STORMAPI SFileSetLocale(LCID lcid);

  // @273
  //BOOL STORMAPI SFileGetBasePath(char* buffer, std::uint32_t bufferchars);

  // @274
  BOOL STORMAPI SFileSetIoErrorMode(std::uint32_t errormode, SFILEERRORPROC errorproc = NULL);

  // @275
  BOOL STORMAPI SFileGetArchiveName(HSARCHIVE archive, char* buffer, std::uint32_t bufferchars);

  // @276
  BOOL STORMAPI SFileGetFileName(HSFILE file, char* buffer, std::uint32_t bufferchars);

  // @277
  BOOL STORMAPI SFileGetArchiveInfo(HSARCHIVE archive, int* priority, BOOL* cdrom);

  // @278
  void SFileSetPlatform(BYTE platform);

  // @279
  BOOL STORMAPI SFileLoadFile(const char* filename, void** buffer, std::uint32_t* buffersize, std::uint32_t extrasize, LPOVERLAPPED lpOverlapped);

  // @280
  BOOL STORMAPI SFileUnloadFile(void* file);

  // @281
  BOOL STORMAPI SFileLoadFileEx(HSARCHIVE hArchive, const char* filename, void** buffer, std::uint32_t* buffersize, std::uint32_t extrasize, std::uint32_t searchScope, struct _OVERLAPPED* lpOverlapped);

  // @282
  //void STORMAPI SFilePrioritizeRequest(int,int);

  // @283
  //void STORMAPI SFileCancelRequest(int);

  // @284
  void STORMAPI SFileSetAsyncBudget(std::uint32_t budget);

  // @285
  void STORMAPI SFileSetDataChunkSize(std::uint32_t chunksize);

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
  void STORMAPI SFileGetInstallPath(char* dest, std::uint32_t destsize, BOOL includeseparator = FALSE);

  // @296
  void STORMAPI SFileGetUserDataPath(char* dest, std::uint32_t destsize, BOOL includeseparator);

  // @297
  void STORMAPI SFileSetUserDataPath(const char* directory);
}

#endif
