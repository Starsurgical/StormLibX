#ifndef __STORMLIBX_NET_SPI_H__
#define __STORMLIBX_NET_SPI_H__

#include <cstdint>
#include <memory>
#include <SNet.h>

struct PROVIDERINFO {
  char     filename[MAX_PATH];
  uint32_t index;
  uint32_t field_20C;
  uint32_t field_210;
  uint32_t id;
  char     desc[SNETSPI_MAXSTRINGLENGTH];
  char     req[SNETSPI_MAXSTRINGLENGTH];
  SNETCAPS caps;
};

struct SPI_SENDBUFFER {
  SNETADDRPTR addrptr[16];
  SNETADDR addr[16];
  uint8_t data[7872];
};

extern std::unique_ptr<SNETSPI> s_spi;

extern uint32_t s_spi_outgoingtime;
extern uint32_t s_spi_timetoresend;
extern PROVIDERINFO* s_spi_providerptr;
extern uint32_t s_spi_timetoackturn;
extern uint32_t s_spi_timetorequest;

int SpiCheckProviderOrder(PROVIDERINFO* first, PROVIDERINFO* second);
void SpiDestroy(BOOL clearproviderlist);
int32_t SpiInitialize(uint32_t providerid, SNETPROGRAMDATAPTR programdata, SNETPLAYERDATAPTR playerdata, SNETUIDATAPTR interfacedata, SNETVERSIONDATAPTR versiondata, void* recvevent);
void* SpiLoadCapsSignature(const char* filename);
BOOL SpiNormalizeDataBlocks(
  SNETPROGRAMDATAPTR programdatain,
  SNETPLAYERDATAPTR playerdatain,
  SNETUIDATAPTR interfacedatain,
  SNETVERSIONDATAPTR versiondatain,
  SNETPROGRAMDATAPTR programdataout,
  SNETPLAYERDATAPTR playerdataout,
  SNETUIDATAPTR interfacedataout,
  SNETVERSIONDATAPTR versiondataout);
BOOL SpiSend(uint32_t addresses, SNETADDRPTR* addrlist, void* data, uint32_t databytes);


#endif
