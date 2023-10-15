#include "SErr.h"

void* s_err_callback;

void SErrSetBlizzardErrorFunction(void* pFn) {
  s_err_callback = pFn;
}

