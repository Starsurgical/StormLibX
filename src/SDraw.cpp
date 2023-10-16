#include "SDraw.h"
#include "SMem.h"

#include <SDL2/SDL.h>
#include <vector>

HWND s_framewindow;
void* s_lpdd;
void* s_surface[4];
BOOL s_surfacelocked;

int s_bpp = 8;
int s_scrwidth = 640;
int s_scrheight = 480;

BYTE* s_bufferbits;

SDL_Window* s_sdl_window;
SDL_Renderer* s_sdl_renderer;

SDL_Surface* s_sdl_surface;
SDL_Surface* s_sdl_canvas;
SDL_Palette* s_sdl_palette;


// @342
BOOL STORMAPI SDrawCaptureScreen(LPCTSTR filename, BOOL usesavepath) {
  return FALSE;
}

// @344
BOOL STORMAPI SDrawClearSurface(int surfacenumber) {
  SDL_Rect rect{ 0, 0, s_scrwidth, s_scrheight };
  if (SDL_FillRect(s_sdl_canvas, &rect, 0) != 0) return FALSE;
  if (SDL_BlitSurface(s_sdl_canvas, nullptr, s_sdl_surface, nullptr) != 0) return FALSE;
  return TRUE;
}

// @345
void STORMAPI SDrawDestroy() {
  SDrawManualInitialize(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
  FREEPTRIFUSED(s_bufferbits);
}

// @346
HWND STORMAPI SDrawGetFrameWindow(HWND* window) {
  if (window) *window = s_framewindow;
  return s_framewindow;
}

// @348
BOOL STORMAPI SDrawGetScreenSize(int* width, int* height, int* bitdepth) {
  if (width) *width = s_scrwidth;
  if (height) *height = s_scrheight;
  if (bitdepth) *bitdepth = s_bpp;
  return width || height || bitdepth;
}

// @350
BOOL STORMAPI SDrawLockSurface(int surfacenumber, LPCRECT rect, LPBYTE* ptr, int* pitch, DWORD flags) {
  if (ptr) *ptr = nullptr;
  if (pitch) *pitch = 0;

  if (s_sdl_canvas == nullptr) return FALSE;
  if (surfacenumber != 0) return FALSE;

  SDL_LockSurface(s_sdl_canvas);

  if (ptr) *ptr = (LPBYTE)s_sdl_canvas->pixels;
  if (pitch) *pitch = s_scrwidth;

  return TRUE;
}

// @351
//BOOL STORMAPI SDrawManualInitialize(HWND framewindow, LPDIRECTDRAW directdraw, LPDIRECTDRAWSURFACE frontbuffer, LPDIRECTDRAWSURFACE backbuffer, LPDIRECTDRAWSURFACE systembuffer, LPDIRECTDRAWSURFACE temporarybuffer);
BOOL STORMAPI SDrawManualInitialize(HWND framewindow, void* directdraw, void* frontbuffer, void* backbuffer, void* systembuffer, void* temporarybuffer) {
  s_framewindow = framewindow;
  s_lpdd = directdraw;
  s_surface[0] = frontbuffer;
  s_surface[1] = backbuffer;
  s_surface[2] = systembuffer;
  s_surface[3] = temporarybuffer;

  if (framewindow) {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

    s_sdl_window = SDL_CreateWindowFrom(framewindow);
    s_sdl_surface = SDL_GetWindowSurface(s_sdl_window);
    s_sdl_canvas = SDL_CreateRGBSurfaceWithFormat(SDL_SWSURFACE, s_scrwidth, s_scrheight, 8, SDL_PIXELFORMAT_INDEX8);
    s_sdl_palette = s_sdl_canvas->format->palette;

    FREEIFUSED(s_bufferbits);
    s_bufferbits = (BYTE*)ALLOC(s_scrwidth * s_scrheight);
  }
  else {
    SDL_FreeSurface(s_sdl_canvas);
    s_sdl_canvas = nullptr;
    s_sdl_palette = nullptr;

    SDL_DestroyWindow(s_sdl_window);
    s_sdl_window = nullptr;
    s_sdl_surface = nullptr;

    SDL_Quit();
  }

  return TRUE;
}

// @352
int STORMAPI SDrawMessageBox(LPCTSTR text, LPCTSTR title, DWORD flags) {
  SDL_MessageBoxData msgbox = {};
  std::vector<SDL_MessageBoxButtonData> buttons;

  if (flags & MB_OKCANCEL) {
    buttons.push_back({ 0, 2, "Cancel" });
    buttons.push_back({ SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 1, "OK" });
  }
  else if (flags & MB_ABORTRETRYIGNORE) {
    buttons.push_back({ 0, 5, "Ignore" });
    buttons.push_back({ 0, 10, "Retry" });
    buttons.push_back({ 0, 3, "Abort" });
  }
  else if (flags & MB_YESNOCANCEL) {
    buttons.push_back({ SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 2, "Cancel" });
    buttons.push_back({ 0, 7, "No" });
    buttons.push_back({ 0, 6, "Yes" });
  }
  else if (flags & MB_YESNO) {
    buttons.push_back({ 0, 7, "No" });
    buttons.push_back({ 0, 6, "Yes" });
  }
  else if (flags & MB_YESNO) {
    buttons.push_back({ SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 2, "Cancel" });
    buttons.push_back({ 0, 10, "Retry" });
  }
  else {
    buttons.push_back({ SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 1, "OK" });
  }
  
  if ((flags & MB_DEFBUTTON3) && buttons.size() >= 3) {
    buttons[buttons.size() - 3].flags |= SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT;
  }
  else if ((flags & MB_DEFBUTTON2) && buttons.size() >= 2) {
    buttons[buttons.size() - 2].flags |= SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT;
  }
  else {
    buttons[buttons.size() - 1].flags |= SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT;
  }

  msgbox.title = title;
  msgbox.message = text;
  msgbox.window = s_sdl_window;
  msgbox.buttons = buttons.data();
  msgbox.numbuttons = int(buttons.size());

  int result = 0;
  SDL_ShowMessageBox(&msgbox, &result);
  return result;
}

// @356
BOOL STORMAPI SDrawUnlockSurface(int surfacenumber, LPBYTE ptr, DWORD numrects, LPCRECT rectarray) {
  if (s_sdl_canvas == nullptr) return FALSE;
  if (surfacenumber != 0) return FALSE;

  SDL_BlitSurface(s_sdl_canvas, nullptr, s_sdl_surface, nullptr);
  return TRUE;
}

// @357
BOOL STORMAPI SDrawUpdatePalette(DWORD firstentry, DWORD numentries, LPPALETTEENTRY entries, BOOL reservedentries) {
  if (s_sdl_palette == nullptr) return FALSE;

  SDL_Color colors[256];
  for (unsigned i = 0; i < numentries; i++) {
    colors[i].r = entries[i].peRed;
    colors[i].g = entries[i].peGreen;
    colors[i].b = entries[i].peBlue;
    colors[i].a = 0;
  }
  SDL_SetPaletteColors(s_sdl_palette, colors, firstentry, numentries);
  SDL_BlitSurface(s_sdl_canvas, nullptr, s_sdl_surface, nullptr);
  return TRUE;
}

