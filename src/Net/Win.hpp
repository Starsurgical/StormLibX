#ifndef __STORMLIBX_NET_WIN_H__
#define __STORMLIBX_NET_WIN_H__

#include <cstdint>

uint32_t PortGetTickCount();
void PortSleep(uint32_t time);
void* PortLoadLibrary(const char* filename);
void PortFreeLibrary(void* object);
void* PortGetProcAddress(void* object, const char* procname);

#endif
