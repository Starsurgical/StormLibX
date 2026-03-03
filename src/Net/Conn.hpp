#ifndef __STORMLIBX_NET_CONN_H__
#define __STORMLIBX_NET_CONN_H__

#include <cstdint>
#include <Storm/List.hpp>
#include <SNet.h>
#include "Pkt.hpp"


#define NOPLAYER 0xFF

#define TYPE_SYSTEM 0
#define TYPE_MESSAGE 1
#define TYPE_TURN 2
#define TYPE_DATAGRAM 3
#define TYPES 4


#define SYS_UNUSED 0
#define SYS_INITIALCONTACT 1
#define SYS_CIRCUITCHECK 2
#define SYS_CIRCUITCHECKRESPONSE 3
#define SYS_PING 4
#define SYS_PINGRESPONSE 5
#define SYS_PLAYERINFO 6
#define SYS_PLAYERJOIN 7
#define SYS_PLAYERJOIN_ACCEPTSTART 8
#define SYS_PLAYERJOIN_ACCEPTDONE 9
#define SYS_PLAYERJOIN_REJECT 10
#define SYS_PLAYERLEAVE 11
#define SYS_DROPPLAYER 12
#define SYS_NEWGAMEOWNER 13
#define SYS_GAMEMODE 14
#define SYSMSGS 16


#define MF_ACK 1
#define MF_RESENDREQUEST 2
#define MF_FORWARDED 4

#define PF_JOINING 4
#define PF_LEAVING 8


struct MESSAGE : TSLinkedNode<MESSAGE>
{
  SNETADDRPTR addr;
  PACKETPTR data;
  uint32_t databytes;
  BOOL local;
  uint32_t sendtime;
  uint32_t resendtime;
};

struct CONNREC : TSLinkedNode<CONNREC>
{
  char name[SNETSPI_MAXSTRINGLENGTH];
  char desc[SNETSPI_MAXSTRINGLENGTH];
  SNETADDR addr;
  uint32_t flags;
  uint32_t lastreceivetime;
  uint32_t lastrequesttime;
  uint32_t lastpingtime;
  uint32_t latency;
  uint32_t peaklatency;
  STORM_LIST(MESSAGE) outgoingqueue[TYPES];
  STORM_LIST(MESSAGE) incomingqueue[TYPES];
  STORM_LIST(MESSAGE) processing[TYPES];
  STORM_LIST(MESSAGE) oldturns;
  uint16_t outgoingsequence[TYPES];
  uint16_t incomingsequence[TYPES];
  uint16_t lastprocessedturn;
  uint16_t availablesequence[TYPES];
  uint16_t acksequence[TYPES];
  uint32_t acktime[TYPES];
  BOOL gameowner;
  BOOL establishing;
  uint32_t field_208;
  uint32_t exitcode;
  uint16_t finalsequence;
  uint8_t playerid;
  uint8_t oldplayerid;
};

extern STORM_LIST(CONNREC) s_conn_local;
extern STORM_LIST(CONNREC) s_conn_connlist;

void ConnAssignPlayerId(CONNREC* conn, uint8_t playerid);
CONNREC* ConnFindByPlayerId(unsigned int playerid);
CONNREC* ConnFindByOldPlayerId(unsigned int playerid);
CONNREC* ConnAddRec(STORM_LIST(CONNREC)* list, SNETADDRPTR addr);
CONNREC* ConnFindByAddr(SNETADDRPTR addr);
CONNREC* ConnFindLocal();
void ConnDestroyQueue(STORM_LIST(MESSAGE)* queue);
void ConnSetCurrentMessage(CONNREC* conn, uint8_t type, MESSAGE* message);
void ConnClearOldTurns(CONNREC* onlyconn);
void ConnFree(CONNREC *conn);
void ConnSendPacket(CONNREC* conn, PACKETPTR pkt);
void ConnProcessAck(CONNREC* conn, uint8_t type, uint16_t acksequence);
void ConnResendMessage(CONNREC* conn, PACKETPTR data, uint32_t databytes);
MESSAGE* ConnSendMessage(CONNREC* target, uint8_t type, uint8_t subtype, void* data, uint32_t databytes);
uint32_t ConnMaintainConnections();
void ConnDestroy();

#endif
