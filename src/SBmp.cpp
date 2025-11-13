#include "SBmp.h"
#include "SMem.h"
#include "SErr.h"
#include "SFile.h"

#include <algorithm>
#include <filesystem>
#include <SDL_image.h>
#include <SDL.h>

namespace {
  const char* DetermineImageType(SDL_RWops *ops) {
    if (IMG_isBMP(ops)) return "BMP";
    if (IMG_isPCX(ops)) return "PCX";
    if (IMG_isPNG(ops)) return "PNG";
    if (IMG_isGIF(ops)) return "GIF";
    return "TGA";
  }
}

// @321
BOOL STORMAPI SBmpDecodeImage(DWORD imagetype, LPBYTE imagedata, DWORD imagebytes, LPPALETTEENTRY paletteentries, LPBYTE bitmapbits, DWORD buffersize, int* width, int* height, int* bitdepth) {
  if (width) *width = 0;
  if (height) *height = 0;
  if (bitdepth) *bitdepth = 0;
  
  VALIDATEBEGIN;
  VALIDATE(buffersize || !bitmapbits);
  VALIDATE(imagedata);
  VALIDATE(imagebytes);
  VALIDATEEND;

  SDL_RWops* ops = SDL_RWFromMem(imagedata, imagebytes);
  SDL_Surface* surface = IMG_LoadTyped_RW(ops, 1, DetermineImageType(ops));

  SDL_LockSurface(surface);

  if (width) *width = surface->w;
  if (height) *height = surface->h;
  if (bitdepth) *bitdepth = surface->format->BitsPerPixel;

  if (bitmapbits && buffersize) {
    DWORD len = std::min(buffersize, static_cast<DWORD>(surface->w * surface->h * surface->format->BytesPerPixel));
    SMemCopy(bitmapbits, surface->pixels, len);
  }

  if (paletteentries && surface->format->palette) {
    int ncolors = std::min(256, surface->format->palette->ncolors);
    for (int i = 0; i < ncolors; i++) {
      SDL_Color& srcpal = surface->format->palette->colors[i];
      PALETTEENTRY& dstpal = paletteentries[i];
      
      dstpal.peRed = srcpal.r;
      dstpal.peGreen = srcpal.g;
      dstpal.peBlue = srcpal.b;
      dstpal.peFlags = 0;
    }
  }

  SDL_UnlockSurface(surface);
  SDL_FreeSurface(surface);
  return TRUE;
}

// @323
BOOL STORMAPI SBmpLoadImage(LPCSTR filename, LPPALETTEENTRY paletteentries, LPBYTE bitmapbits, DWORD buffersize, int* width, int* height, int* bitdepth) {
  if (width) *width = 0;
  if (height) *height = 0;
  if (bitdepth) *bitdepth = 0;

  VALIDATEBEGIN;
  VALIDATE(buffersize || !bitmapbits);
  VALIDATE(filename);
  VALIDATEEND;

  HSFILE hFile;
  if (!SFileOpenFile(filename, &hFile)) return FALSE;

  DWORD imagebytes = SFileGetFileSize(hFile);
  LPBYTE imagedata = static_cast<LPBYTE>(ALLOC(imagebytes));
  if (!SFileReadFile(hFile, imagedata, imagebytes)) {
    SFileCloseFile(hFile);
    return FALSE;
  }
  SFileCloseFile(hFile);

  SBmpDecodeImage(0, imagedata, imagebytes, paletteentries, bitmapbits, buffersize, width, height, bitdepth);
  return TRUE;
}

// @324
BOOL STORMAPI SBmpSaveImage(LPCSTR filename, LPPALETTEENTRY paletteentries, LPBYTE bitmapbits, int width, int height, int bitdepth) {
  return SBmpSaveImageEx(filename, paletteentries, bitmapbits, width, height, bitdepth, 0);
}

// @325
BOOL STORMAPI SBmpAllocLoadImage(LPCSTR filename, LPPALETTEENTRY paletteentries, LPBYTE* returnedbuffer, int* width, int* height, int* bitdepth, int requestedbitdepth, SBMPALLOCPROC allocproc) {
  if (returnedbuffer) *returnedbuffer = nullptr;
  if (width) *width = 0;
  if (height) *height = 0;
  if (bitdepth) *bitdepth = 0;

  VALIDATEBEGIN;
  VALIDATE(filename);
  VALIDATE(returnedbuffer);
  VALIDATEEND;

  HSFILE hFile;
  if (!SFileOpenFile(filename, &hFile)) return FALSE;

  DWORD imagebytes = SFileGetFileSize(hFile);
  LPBYTE imagedata = static_cast<LPBYTE>(ALLOC(imagebytes));
  if (!SFileReadFile(hFile, imagedata, imagebytes)) {
    SFileCloseFile(hFile);
    return FALSE;
  }
  SFileCloseFile(hFile);

  int w, h, bpp;
  SBmpDecodeImage(0, imagedata, imagebytes, nullptr, nullptr, 0, &w, &h, &bpp);

  DWORD pixeldatasize = w * h * bpp / 8;
  LPBYTE pixeldata = static_cast<LPBYTE>(allocproc ? allocproc(pixeldatasize) : ALLOC(pixeldatasize));
  return SBmpDecodeImage(0, imagedata, imagebytes, paletteentries, pixeldata, pixeldatasize, width, height, bitdepth);
}

// @326
BOOL STORMAPI SBmpSaveImageEx(LPCSTR filename, LPPALETTEENTRY paletteentries, LPBYTE bitmapbits, int width, int height, int bitdepth, int alignment) {
  if (bitdepth != 8 && bitdepth != 24 && bitdepth != 32) {
    return FALSE;
  }

  Uint32 pixelformat;
  switch (bitdepth) {
  case 8: pixelformat = SDL_PIXELFORMAT_INDEX8; break;
  case 24: pixelformat = SDL_PIXELFORMAT_RGB888; break;
  case 32: pixelformat = SDL_PIXELFORMAT_RGBA8888; break;
  }
  SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormatFrom(bitmapbits, width, height, bitdepth, width * bitdepth / 8, pixelformat);

  auto extension = std::filesystem::path(filename).extension();
  BOOL result = FALSE;
  
  if (extension == ".png") {
    result = IMG_SavePNG(surface, filename) == 0;
  }
  else if (extension == ".bmp") {
    result = SDL_SaveBMP(surface, filename) == 0;
  }

  SDL_FreeSurface(surface);
  return result;
}

// @327
BOOL STORMAPI SBmpPadImage(LPBYTE imagedata, DWORD imagebytes, int width, int height, int alignment, DWORD* out_newwidth) {
  if (width % alignment == 0) return TRUE;

  int newwidth = SBmpGetPitchForAlignment(width, alignment);

  BYTE* newptr = imagedata;
  BYTE* oldptr = &imagedata[height * newwidth - height * width];

  SMemMove(oldptr, newptr, height * width);

  for (; height > 0; height--) {
    SMemMove(newptr, oldptr, width);
    SMemZero(&newptr[width], newwidth - width);
    oldptr += width;
    newptr += newwidth;
  }

  if (out_newwidth) *out_newwidth = newwidth;
  return TRUE;
}

// @328
int STORMAPI SBmpGetPitchForAlignment(int width, int align) {
  int result = width;
  if (width % align != 0) {
    result = align + width - width % align;
  }
  return result;
}
