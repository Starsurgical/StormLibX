#include "Win.hpp"

#include <chrono>
#include <thread>

#include <SDL.h>
#include <SDL_loadso.h>


uint32_t PortGetTickCount() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

void PortSleep(uint32_t time) {
  std::this_thread::sleep_for(std::chrono::milliseconds(time));
}

void* PortLoadLibrary(const char* filename) {
  return SDL_LoadObject(filename);
}

void PortFreeLibrary(void* object) {
  SDL_UnloadObject(object);
}

void* PortGetProcAddress(void* object, const char* procname) {
  return SDL_LoadFunction(object, procname);
}
