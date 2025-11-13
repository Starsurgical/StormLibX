#include "SLog.h"

static int s_sequence;


// @541
void STORMAPI SLogClose(HSLOG log) {
}

// @542
BOOL STORMAPI SLogCreate(const char *filename, BYTE flags, HSLOG *log) {
  return TRUE;
}

// @548
void STORMCDECL SLogWrite(HSLOG log, const char *format, ...) {
}

// @550
void STORMAPI SLogSetIndent(HSLOG log, int indent) {
}
