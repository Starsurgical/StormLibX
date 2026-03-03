#include "Spi.hpp"
#include <memory>
#include <vector>

#include <SFile.h>
#include <SDraw.h>
#include <SMem.h>

#include "Perf.hpp"
#include "Win.hpp"

uint32_t s_spi_outgoingtime;
bool s_spi_providersfound;
std::unique_ptr<SPI_SENDBUFFER> s_spi_sendbuffer;
void* s_spi_lib;
std::unique_ptr<SNETSPI> s_spi;
std::vector<PROVIDERINFO> s_spi_providerlist;
PROVIDERINFO* s_spi_providerptr;

uint32_t s_spi_timetoackturn = 250;
uint32_t s_spi_timetoblock = 5000;
uint32_t s_spi_timetogiveup = 1000;
uint32_t s_spi_timetorequest = 25;
uint32_t s_spi_timetoresend = 50;


static int STORMAPI SMessageBox(HWND hWnd, const char* lpText, const char* lpCaption, UINT uType) {
  return SDrawMessageBox(lpText, lpCaption, uType);
}

int SpiCheckProviderOrder(PROVIDERINFO* first, PROVIDERINFO* second) {
  static const uint32_t baseorder[] = {'BNET', 'IPXN', 'IPXW', 'MODM', 'SCBL', 'MSDP'};
  int firstindex, secondindex;
  firstindex = secondindex = std::numeric_limits<int>::max();

  for (int i = 0; i < std::size(baseorder); i++) {
    if (first->id == baseorder[i]) firstindex = i;
    if (second->id == baseorder[i]) secondindex = i;
  }

  if (firstindex != std::numeric_limits<int>::max() || secondindex != std::numeric_limits<int>::max()) {
    return secondindex - firstindex;
  }
  return SStrCmpI(first->desc, second->desc);
}

void SpiDestroy(BOOL clearproviderlist) {
  if (s_spi_lib && s_spi) {
    s_spi->spiDestroy();
  }

  if (s_spi_lib) {
    PortFreeLibrary(s_spi_lib);
    s_spi_lib = nullptr;
  }

  s_spi.reset();
  s_spi_sendbuffer.reset();

  if (clearproviderlist) {
    s_spi_providerlist.clear();
    s_spi_providersfound = false;
  }
  s_spi_providerptr = nullptr;
}

// made up to simplify code
bool SpiDestroyWithError(uint32_t errorcode) {
  SpiDestroy(false);
  SErrSetLastError(errorcode);
  return false;
}

int32_t SpiInitialize(uint32_t providerid, SNETPROGRAMDATAPTR programdata, SNETPLAYERDATAPTR playerdata, SNETUIDATAPTR interfacedata, SNETVERSIONDATAPTR versiondata, void* recvevent) {
  SpiDestroy(false);
  //SpiFindAllProviders();
  s_spi_providerptr = nullptr;
  for (PROVIDERINFO& provider : s_spi_providerlist) {
    if (provider.id == providerid) {
      s_spi_providerptr = &provider;
      break;
    }
  }

  if (!s_spi_providerptr) return SpiDestroyWithError(ERROR_BAD_PROVIDER);

  s_spi_timetoackturn = std::max(5 * s_spi_providerptr->caps.latencyms, 250u);
  s_spi_timetoblock = std::max(12 * s_spi_providerptr->caps.latencyms, 5000u);
  s_spi_timetogiveup = std::max(4 * s_spi_providerptr->caps.latencyms, 1000u);
  s_spi_timetorequest = std::max(s_spi_providerptr->caps.latencyms / 2, 25u);
  s_spi_timetoresend = std::max(s_spi_providerptr->caps.latencyms, 50u);
  
  if (s_spi_providerptr->caps.flags & SNET_CAPS_PAGELOCKEDBUFFERS) {
    s_spi_sendbuffer.reset(new SPI_SENDBUFFER{ 0 });
    if (!s_spi_sendbuffer) return SpiDestroyWithError(ERROR_NOT_ENOUGH_MEMORY);
    //VirtualLock(s_spi_sendbuffer.get(), sizeof(SPI_SENDBUFFER));
  }

  s_spi_lib = PortLoadLibrary(s_spi_providerptr->filename);
  if (!s_spi_lib) return SpiDestroyWithError(ERROR_BAD_PROVIDER);

  SNETSPIBIND bind = static_cast<SNETSPIBIND>(PortGetProcAddress(s_spi_lib, "SnpBind"));
  if (!bind) return SpiDestroyWithError(ERROR_BAD_PROVIDER);

  SNETSPIPTR returnedspi = nullptr;
  bind(s_spi_providerptr->index, &returnedspi);
  if (!returnedspi || returnedspi->size < 80) return SpiDestroyWithError(ERROR_BAD_PROVIDER);

  s_spi.reset(new SNETSPI{});
  SMemCopy(s_spi.get(), returnedspi, std::min(static_cast<uint32_t>(sizeof(SNETSPI)), returnedspi->size));

  if (!s_spi->spiInitialize(programdata, playerdata, interfacedata, versiondata, recvevent)) {
    return SpiDestroyWithError(SErrGetLastError());
  }
  return true;
}

void* SpiLoadCapsSignature(const char* filename) {
  void* result = nullptr;

  HSARCHIVE archive = nullptr;
  HSFILE file = nullptr;

  if (SFileOpenArchive(filename, 0, 0, &archive)) {
    uint32_t authtype;
    SFileAuthenticateArchive(archive, &authtype);
    if (authtype == SFILE_AUTH_UNABLETOAUTHENTICATE || authtype >= SFILE_AUTH_AUTHENTICBLIZZARD) {
      if (SFileOpenFileEx(archive, "caps.dat", 0, &file)) {
        uint32_t bytes = SFileGetFileSize(file);
        result = ALLOC(bytes);
        SFileReadFile(file, result, bytes);
      }
    }
  }

  if (file) SFileCloseFile(file);
  if (archive) SFileCloseArchive(archive);
  return result;
}

BOOL SpiNormalizeDataBlocks(
  SNETPROGRAMDATAPTR programdatain,
  SNETPLAYERDATAPTR playerdatain,
  SNETUIDATAPTR interfacedatain,
  SNETVERSIONDATAPTR versiondatain,
  SNETPROGRAMDATAPTR programdataout,
  SNETPLAYERDATAPTR playerdataout,
  SNETUIDATAPTR interfacedataout,
  SNETVERSIONDATAPTR versiondataout) {

  *programdataout = *programdatain;
  programdataout->size = sizeof(SNETPROGRAMDATA);
  if (!programdataout->programname) programdataout->programname = "";

  if (!programdataout->programdescription) {
    programdataout->programdescription = "";  // TODO program description
  }

  if (!programdataout->cdkey) programdataout->cdkey = "";

  *playerdataout = *playerdatain;
  playerdataout->size = sizeof(SNETPLAYERDATA);
  if (!playerdataout->playername) playerdataout->playername = "";
  if (!playerdataout->playerdescription) playerdataout->playerdescription = "";

  *interfacedataout = *interfacedatain;
  interfacedataout->size = sizeof(INTERFACEDATA);
  if (!interfacedataout->parentwindow) {
    interfacedataout->parentwindow = SDrawGetFrameWindow();
  }
  if (!interfacedataout->messageboxcallback) {
    interfacedataout->messageboxcallback = SMessageBox;
  }

  *versiondataout = *versiondatain;
  versiondataout->size = sizeof(SNETVERSIONDATA);
  return TRUE;
}

BOOL SpiSend(uint32_t addresses, SNETADDRPTR* addrlist, void* data, uint32_t databytes) {
  if (!s_spi) return FALSE;

  uint32_t currtime = PortGetTickCount();
  if (currtime - s_spi_outgoingtime < std::numeric_limits<int32_t>::max()) {
    s_spi_outgoingtime = currtime;
  }
  s_spi_outgoingtime += 1000 * addresses * (databytes + 64) / s_spi_providerptr->caps.bytessec;

  if (s_spi_sendbuffer) {
    if (addresses > 16 || databytes > sizeof(s_spi_sendbuffer->data)) {
      return FALSE;
    }

    for (uint32_t i = 0; i < addresses; i++) {
      s_spi_sendbuffer->addrptr[i] = &s_spi_sendbuffer->addr[i];
    }

    for (uint32_t i = 0; i < addresses; i++) {
      s_spi_sendbuffer->addr[i] = *addrlist[i];
    }

    SMemCopy(s_spi_sendbuffer->data, data, databytes);
    data = s_spi_sendbuffer->data;
  }
  PerfAdd(SNET_PERFID_TOTALBYTESSENT, databytes);
  return s_spi->spiSend(addresses, addrlist, data, databytes);
}
