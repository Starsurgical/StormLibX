#ifndef __STORMLIBX_NET_SYS_H__
#define __STORMLIBX_NET_SYS_H__

#include <cstdint>
#include <SNet.h>

struct CONNREC;

typedef struct _SYSEVENT {
  SNETADDRPTR senderaddr;
  void*       data;
  uint32_t    databytes;
  uint8_t     senderplayerid;
  uint8_t     eventid;
} SYSEVENT, *SYSEVENTPTR;

typedef struct _SYSEVENTDATA_DROPPLAYER {
  uint32_t playerid;
  uint32_t finalsequence;
  uint32_t exitcode;
} SYSEVENTDATA_DROPPLAYER, *SYSEVENTDATA_DROPPLAYERPTR;

typedef struct _SYSEVENTDATA_PLAYERJOIN {
  char namedescpass[3*SNETSPI_MAXSTRINGLENGTH];
} SYSEVENTDATA_PLAYERJOIN, *SYSEVENTDATA_PLAYERJOINPTR;

typedef struct _SYSEVENTDATA_PLAYERJOIN_ACCEPTSTART {
  uint32_t playerid;
  uint32_t playersallowed;
  uint32_t nextturn;
  uint32_t gamemode;
  uint32_t runningtime;
  char     namedescpass[3*SNETSPI_MAXSTRINGLENGTH];
} SYSEVENTDATA_PLAYERJOIN_ACCEPTSTART, *SYSEVENTDATA_PLAYERJOIN_ACCEPTSTARTPTR;

typedef struct _SYSEVENTDATA_PLAYERINFO {
  uint32_t bytes;
  uint32_t playerid;
  BOOL     gameowner;
  uint32_t flags;
  uint32_t startingturn;
  SNETADDR addr;
  char     namedesc[SNETSPI_MAXSTRINGLENGTH*2];
} SYSEVENTDATA_PLAYERINFO, *SYSEVENTDATA_PLAYERINFOPTR;

typedef struct _SYSEVENTDATA_PLAYERLEAVE {
  uint32_t finalsequence;
  uint32_t exitcode;
} SYSEVENTDATA_PLAYERLEAVE, *SYSEVENTDATA_PLAYERLEAVEPTR;

struct USEREVENT : TSLinkedNode<USEREVENT> {
  SNETEVENT event;
};

uint32_t SysBuildPlayerInfo(SYSEVENTDATA_PLAYERINFOPTR data, CONNREC* conn, uint32_t startingturn);
void STORMAPI SysOnCircuitCheck(SYSEVENTPTR event);
void STORMAPI SysOnDropPlayer(SYSEVENTPTR event);
void STORMAPI SysOnNewGameMode(SYSEVENTPTR event);
void STORMAPI SysOnNewGameOwner(SYSEVENTPTR event);
void STORMAPI SysOnPing(SYSEVENTPTR event);
void STORMAPI SysOnPingResponse(SYSEVENTPTR event);
void SysQueueUserEvent(uint32_t eventid, uint32_t playerid, const void* data, uint32_t databytes);
void STORMAPI SysOnPlayerJoinAcceptDone(SYSEVENTPTR event);
void STORMAPI SysOnPlayerJoinAcceptStart(SYSEVENTPTR event);
void STORMAPI SysOnPlayerLeave(SYSEVENTPTR event);
void STORMAPI SysOnNewLadderId(SYSEVENTPTR event);
void STORMAPI SysOnPlayerJoinReject(SYSEVENTPTR event);
void SysDestroy();
void SysDispatchUserEvents();
int32_t SysWaitForMultipleEvents(uint32_t numevents, uint32_t* eventlist, int32_t waitforall, uint32_t timeout);


#endif
