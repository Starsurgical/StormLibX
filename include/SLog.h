#ifndef __STORMLIBX_SLOG_H__
#define __STORMLIBX_SLOG_H__

#include "StormTypes.h"


DECLARE_HANDLE(HSLOCKEDLOG);
DECLARE_HANDLE(HSLOG);

// @541
void STORMAPI SLogClose(HSLOG log);

// @542
BOOL STORMAPI SLogCreate(const char *filename, BYTE flags, HSLOG *log);

// @548
void STORMCDECL SLogWrite(HSLOG log, const char *format, ...);

// @550
void STORMAPI SLogSetIndent(HSLOG log, int indent);


#endif
