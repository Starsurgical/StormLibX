#include "Pkt.hpp"
#include <SMem.h>


uint16_t PktComputeChecksum(void* data, uint32_t databytes) {
  uint32_t checkval1 = 0;
  uint32_t checkval2 = 0;

  uint8_t *ptr = static_cast<uint8_t*>(data);
  while (databytes--) {
    checkval1 += ptr[databytes];
    if (checkval1 >= 255) checkval1 -= 255;
    checkval2 += checkval1;
  }
  return (uint8_t(checkval1) << 8) | uint8_t(checkval2 + checkval2 / 255);
}

uint16_t PktGenerateChecksum(PACKET* pkt) {
  uint16_t chk = PktComputeChecksum(&pkt->header.bytes, pkt->header.bytes - 2);
  uint8_t hibyte = 255 - chk % 255;
  uint8_t lobyte = 255 - hibyte % 255;
  return (hibyte << 8) | lobyte;
}

static void PktFreeLocalMessage(SNETADDRPTR addr, PACKETPTR data) {
  if (addr) delete addr;
  FREEIFUSED(data);
}

static void PktAllocateLocalMessage(SNETADDRPTR* addr, PACKETPTR* data, uint32_t databytes) {
  *addr = STORM_NEW(SNETADDR);
  
  databytes = databytes + (sizeof(intptr_t) - databytes % sizeof(intptr_t));
  if (databytes < sizeof(intptr_t)) databytes = sizeof(intptr_t);
  *data = static_cast<PACKETPTR>(ALLOC(databytes));
}
