#ifndef __STORMLIBX_NET_PKT_H__
#define __STORMLIBX_NET_PKT_H__

#include <cstdint>
#include <SNet.h>

struct HEADER
{
  uint16_t checksum;
  uint16_t bytes;
  uint16_t sequence;
  uint16_t acksequence;
  uint8_t  type;
  uint8_t  subtype;
  uint8_t  playerid;
  uint8_t  flags;
};

typedef struct PACKET
{
  HEADER header;
  uint8_t data[];
} *PACKETPTR;

uint16_t PktComputeChecksum(void* data, uint32_t databytes);
uint16_t PktGenerateChecksum(PACKET* pkt);
static void PktFreeLocalMessage(SNETADDRPTR addr, PACKETPTR data);
static void PktAllocateLocalMessage(SNETADDRPTR* addr, PACKETPTR* data, uint32_t databytes);

#endif
