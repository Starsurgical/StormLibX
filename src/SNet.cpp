#include <storm/Memory.hpp>
#include <storm/Event.hpp>
#include "SDraw.h"
#include "SErr.h"
#include "SFile.h"
#include "SMem.h"
#include "SNet.h"

#include <SDL.h>
#include <SDL_loadso.h>

#include <atomic>
#include <chrono>
#include <filesystem>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#define CONCAT(a, b) CONCAT_INNER(a, b)
#define CONCAT_INNER(a, b) a ## b
#define SCOPE_LOCK(x) std::lock_guard<decltype(x)> CONCAT(x, __COUNTER__)(x)

/*
caps.dat

  struct {
    uint32_t entrysize;
    uint32_t providerid;
    char description[]; // null terminated string of any length
    SNETCAPS caps;
  }[]; // repeats for the number of providers
*/

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

#define SNET_PERFID_TURN 1
#define SNET_PERFID_TURNSSENT 4
#define SNET_PERFID_TURNSRECV 5
#define SNET_PERFID_MSGSENT 6
#define SNET_PERFID_MSGRECV 7
#define SNET_PERFID_USERBYTESSENT 8
#define SNET_PERFID_USERBYTESRECV 9
#define SNET_PERFID_TOTALBYTESSENT 10
#define SNET_PERFID_TOTALBYTESRECV 11
#define SNET_PERFID_PKTSENTONWIRE 12
#define SNET_PERFID_PKTRECVONWIRE 13
#define SNET_PERFID_BYTESSENTONWIRE 14
#define SNET_PERFID_BYTESRECVONWIRE 15
#define SNET_PERFIDNUM 16

#define SNET_PERFTYPE_COUNTER 0x10410400
#define SNET_PERFTYPE_RAWCOUNT 0x00010000


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
  char name[128];
  char desc[128];
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

struct _PLAYERNAME {
  char name[128];
};

struct USEREVENT : TSLinkedNode<USEREVENT> {
  SNETEVENT event;
};

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

struct SPI_SENDBUFFER {
  SNETADDRPTR addrptr[16];
  SNETADDR addr[16];
  uint8_t data[7872];
};

typedef struct _CLIENTDATA {
  uint32_t bytes;
  uint32_t numplayers;
  uint32_t maxplayers;
} CLIENTDATA, *CLIENTDATAPTR;

static uint32_t s_spi_outgoingtime;
static bool s_spi_providersfound;
static std::unique_ptr<SPI_SENDBUFFER> s_spi_sendbuffer;

static std::recursive_mutex s_api_critsect;
static std::recursive_mutex s_sys_usereventlist_critsect;

static void* s_spi_lib;
static std::unique_ptr<SNETSPI> s_spi;
static std::vector<PROVIDERINFO> s_spi_providerlist;
static PROVIDERINFO* s_spi_providerptr;
static uint32_t s_api_playeroffset;

static CODEVERIFYPROC s_CodeSignFunc;

static uint32_t s_spi_timetoackturn = 250;
static uint32_t s_spi_timetoblock = 5000;
static uint32_t s_spi_timetogiveup = 1000;
static uint32_t s_spi_timetorequest = 25;
static uint32_t s_spi_timetoresend = 50;
static uint8_t s_game_playerid = NOPLAYER;
static std::thread s_recv_thread;
static std::atomic_bool s_recv_shutdown;
static HANDLE s_recv_event;  // FIXME: move off Windows

static STORM_LIST(CONNREC) s_conn_local;
static STORM_LIST(CONNREC) s_conn_connlist;
static STORM_LIST(USEREVENT) s_sys_usereventlist;

static std::vector<_PLAYERNAME> s_game_playernames;
static uint32_t s_game_categorybits;
static uint32_t s_game_creationtime;
static void* s_game_initdata;
static uint32_t s_game_initdatabytes;
static uint32_t s_game_gamemode;
static char s_game_gamedesc[128];
static char s_game_gamename[128];
static char s_game_gamepass[128];
static uint32_t s_game_joining;
static uint32_t s_game_ladderid;
static uint32_t s_game_optcategorybits;
static uint32_t s_game_playersallowed;
static uint32_t s_game_programid;
static uint32_t s_game_versionid;

static bool s_sys_event[SYSMSGS];


typedef struct _PERFDATAREC {
  std::atomic_uint32_t value;
  uint32_t type;
  int32_t scale;
  bool providerspecific;
} PERFDATAREC, *PERFDATAPTR;

static PERFDATAREC s_perf_data[SNET_PERFIDNUM] = {
  {},
  { 0, SNET_PERFTYPE_RAWCOUNT, -1, false },
  {},
  {},
  { 0, SNET_PERFTYPE_COUNTER, -1, false },
  { 0, SNET_PERFTYPE_COUNTER, -1, false },
  { 0, SNET_PERFTYPE_COUNTER, -1, false },
  { 0, SNET_PERFTYPE_COUNTER, -1, false },
  { 0, SNET_PERFTYPE_COUNTER, -4, false },
  { 0, SNET_PERFTYPE_COUNTER, -4, false },
  { 0, SNET_PERFTYPE_COUNTER, -4, false },
  { 0, SNET_PERFTYPE_COUNTER, -4, false },
  { 0, SNET_PERFTYPE_COUNTER, -1, true },
  { 0, SNET_PERFTYPE_COUNTER, -1, true },
  { 0, SNET_PERFTYPE_COUNTER, -4, true },
  { 0, SNET_PERFTYPE_COUNTER, -4, true },
};

static void PerfAdd(uint32_t id, int32_t value) {
  s_perf_data[id].value += value;
}

static void PerfIncrement(uint32_t id) {
  s_perf_data[id].value++;
}

static void PerfSet(uint32_t id, int32_t value) {
  s_perf_data[id].value = value;
}

static BOOL SpiSend(uint32_t addresses, SNETADDRPTR* addrlist, void* data, uint32_t databytes);

static uint32_t PortGetTickCount() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

static void PortSleep(uint32_t time) {
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

static void GameBuildClientData(CLIENTDATAPTR buffer, uint32_t* bytes) {
  buffer->bytes = sizeof(CLIENTDATA);
  SNetGetNumPlayers(nullptr, nullptr, &buffer->numplayers);
  buffer->maxplayers = s_game_playersallowed;
  *bytes = buffer->bytes;
}

static void GameSetPlayerName(unsigned int id, const char *name) {
  STORM_VALIDATE_BEGIN;
  STORM_VALIDATE(id < 256);
  STORM_VALIDATE_END;

  if (id + 1 > s_game_playernames.size()) {
    s_game_playernames.resize(id + 1, {});
  }

  SStrCopy(s_game_playernames[id].name, name, sizeof(_PLAYERNAME::name));
}

static BOOL GameStartAdvertising() {
  CLIENTDATA clientdata;
  uint32_t clientdatabytes;
  GameBuildClientData(&clientdata, &clientdatabytes);
  uint32_t gameage = (PortGetTickCount() - s_game_creationtime) / 1000;
  return s_spi->spiStartAdvertisingGame(s_game_gamename, s_game_gamepass, s_game_gamedesc, s_game_gamemode, gameage, s_game_categorybits, s_game_optcategorybits, s_game_ladderid, &clientdata, clientdatabytes);
}

static void ConnAssignPlayerId(CONNREC* conn, uint8_t playerid) {
  conn->oldplayerid = NOPLAYER;
  conn->playerid = playerid;
  
  for (CONNREC* curr = s_conn_local.Head(); curr; curr = curr->Next()) {
    if (curr->oldplayerid == playerid) {
      curr->oldplayerid = NOPLAYER;
    }
  }
  
  for (CONNREC* curr = s_conn_connlist.Head(); curr; curr = curr->Next()) {
    if (curr->oldplayerid == playerid) {
      curr->oldplayerid = NOPLAYER;
    }
  }
}

static CONNREC* ConnFindByPlayerId(unsigned int playerid) {
  if (playerid == 255) return nullptr;

  for (CONNREC* curr = s_conn_local.Head(); curr; curr = curr->Next()) {
    if (curr->playerid == playerid) {
      return curr;
    }
  }

  for (CONNREC* curr = s_conn_connlist.Head(); curr; curr = curr->Next()) {
    if (curr->playerid == playerid) {
      return curr;
    }
  }
  return nullptr;
}

static CONNREC* ConnFindByOldPlayerId(unsigned int playerid) {
  if (playerid == 255) return nullptr;

  for (CONNREC* curr = s_conn_local.Head(); curr; curr = curr->Next()) {
    if (curr->oldplayerid == playerid && curr->playerid == NOPLAYER) {
      return curr;
    }
  }

  for (CONNREC* curr = s_conn_connlist.Head(); curr; curr = curr->Next()) {
    if (curr->oldplayerid == playerid && curr->playerid == NOPLAYER) {
      return curr;
    }
  }
  return nullptr;
}

static CONNREC* ConnAddRec(STORM_LIST(CONNREC)* list, SNETADDRPTR addr) {
  CONNREC *newconn = list->NewNode(STORM_LIST_TAIL, 0, 0);
  newconn->addr = *addr;
  newconn->playerid = NOPLAYER;
  newconn->oldplayerid = NOPLAYER;
  newconn->lastreceivetime = PortGetTickCount();
  return newconn;
}

static CONNREC* ConnFindByAddr(SNETADDRPTR addr) {
  for (CONNREC *curr = s_conn_connlist.Head(); curr; curr = curr->Next()) {
    if (memcmp(&curr->addr, addr, sizeof(SNETADDR)) == 0) {
      return curr;
    }
  }
  return ConnAddRec(&s_conn_connlist, addr);
}

static CONNREC* ConnFindLocal() {
  if (!s_conn_local.IsEmpty()) {
    return s_conn_local.Head();
  }

  SNETADDR addr = {};
  return ConnAddRec(&s_conn_local, &addr);
}

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

static void ConnDestroyQueue(STORM_LIST(MESSAGE)* queue) {
  while (MESSAGE *currmsg = queue->Head()) {
    if (currmsg->local) {
      PktFreeLocalMessage(currmsg->addr, currmsg->data);
    }
    else if (s_spi) {
      s_spi->spiFree(currmsg->addr, currmsg->data, currmsg->databytes);
    }
    queue->DeleteNode(currmsg);
  }
}

static void ConnSetCurrentMessage(CONNREC* conn, uint8_t type, MESSAGE* message) {
  if (type == TYPE_TURN) {
    conn->oldturns.LinkNode(message, STORM_LIST_TAIL, nullptr);
    return;
  }

  if (!conn->processing[type].IsEmpty()) {
    MESSAGE* processing = conn->processing[type].Head();
    if (processing->local) {
      PktFreeLocalMessage(processing->addr, processing->data);
    }
    else {
      s_spi->spiFree(processing->addr, processing->data, processing->databytes);
    }
    conn->processing[type].DeleteNode(processing);
  }
  conn->processing[type].LinkNode(message, STORM_LIST_TAIL, nullptr);
}

static void ConnClearOldTurns(CONNREC* onlyconn) {
  CONNREC* localptr = ConnFindLocal();
  if (!localptr) return;

  uint16_t sequence = localptr->incomingsequence[TYPE_TURN];
  uint16_t acksequence = sequence;
  for (CONNREC* conn = s_conn_connlist.Head(); conn; conn = conn->Next()) {
    if (acksequence - conn->lastprocessedturn < INT16_MAX) {
      acksequence = conn->lastprocessedturn;
    }
  }

  for (int32_t local = 0; local <= 1; local++) {
    CONNREC* connhead = local ? localptr : s_conn_connlist.Head();
    for (CONNREC* conn = connhead; conn; conn = conn->Next()) {
      if (onlyconn && conn != onlyconn) continue;

      while (!conn->incomingqueue[TYPE_TURN].IsEmpty()) {
        if (sequence == conn->incomingqueue[TYPE_TURN].Head()->data->header.sequence) break;
        if (sequence - conn->incomingqueue[TYPE_TURN].Head()->data->header.sequence > INT16_MAX) break;

        MESSAGE* message = conn->incomingqueue[TYPE_TURN].Head();
        conn->incomingqueue[TYPE_TURN].UnlinkNode(message);
        ConnSetCurrentMessage(conn, TYPE_TURN, message);
      }

      while(!conn->oldturns.IsEmpty()) {
        if (acksequence == conn->oldturns.Head()->data->header.sequence) break;
        if (acksequence - conn->oldturns.Head()->data->header.sequence > INT16_MAX) break;

        MESSAGE* message = conn->oldturns.Head();
        if (local) {
          PktFreeLocalMessage(message->addr, message->data);
        }
        else {
          s_spi->spiFree(message->addr, message->data, message->databytes);
          conn->oldturns.DeleteNode(message);
        }
      }
    }
  }
}

static void ConnFree(CONNREC *conn) {
  for (int i = 0; i < TYPES; i++) {
    ConnDestroyQueue(&conn->outgoingqueue[i]);
    ConnDestroyQueue(&conn->incomingqueue[i]);
    ConnDestroyQueue(&conn->processing[i]);
  }
  ConnDestroyQueue(&conn->oldturns);
  delete conn;
}

static void ConnSendPacket(CONNREC* conn, PACKETPTR pkt) {
  SNETADDRPTR addr = &conn->addr;
  SpiSend(1, &addr, pkt, pkt->header.bytes);
  conn->acksequence[pkt->header.type] = pkt->header.acksequence;
  conn->acktime[pkt->header.type] = 0;
}

static void ConnProcessAck(CONNREC* conn, uint8_t type, uint16_t acksequence) {
  bool found = false;
  while (!conn->outgoingqueue[type].IsEmpty()) {
    if (acksequence == conn->outgoingqueue[type].Head()->data->header.sequence) break;
    if (acksequence - conn->outgoingqueue[type].Head()->data->header.sequence > INT16_MAX) break;

    found = true;
    MESSAGE* message = conn->outgoingqueue[type].Head();
    if (message->local) {
      PktFreeLocalMessage(message->addr, message->data);
    }
    else if (s_spi) {
      s_spi->spiFree(message->addr, message->data, message->databytes);
    }
    conn->outgoingqueue[type].DeleteNode(message);
  }

  if (type == TYPE_TURN) {
    conn->lastprocessedturn = acksequence;
  }

  if (found && !conn->outgoingqueue[type].IsEmpty()) {
    uint32_t currtime = PortGetTickCount();
    if (currtime - conn->outgoingqueue[type].Head()->resendtime < INT32_MAX) {
      conn->outgoingqueue[type].Head()->resendtime = currtime;
    }
  }
}

static void ConnResendMessage(CONNREC* conn, PACKETPTR data, uint32_t databytes) {
  PACKETPTR localpkt = static_cast<PACKETPTR>(ALLOC(databytes));
  SMemCopy(localpkt, data, databytes);

  uint32_t bytes = localpkt->header.bytes;
  localpkt->header.acksequence = conn->availablesequence[localpkt->header.type];
  localpkt->header.checksum = PktGenerateChecksum(localpkt);

  ConnSendPacket(conn, localpkt);
  FREE(localpkt);
}

static void PktAllocateLocalMessage(SNETADDRPTR* addr, PACKETPTR* data, uint32_t databytes) {
  *addr = STORM_NEW(SNETADDR);
  
  databytes = databytes + (sizeof(intptr_t) - databytes % sizeof(intptr_t));
  if (databytes < sizeof(intptr_t)) databytes = sizeof(intptr_t);
  *data = static_cast<PACKETPTR>(ALLOC(databytes));
}

static MESSAGE* ConnSendMessage(CONNREC* target, uint8_t type, uint8_t subtype, void* data, uint32_t databytes) {
  CONNREC* local = ConnFindLocal();
  if (!local) return nullptr;

  SNETADDRPTR pktaddr = nullptr;
  PACKETPTR pkt = nullptr;
  PktAllocateLocalMessage(&pktaddr, &pkt, databytes + sizeof(HEADER));
  *pktaddr = target->addr;

  pkt->header.bytes = databytes + sizeof(HEADER);
  pkt->header.acksequence = target->availablesequence[type];
  pkt->header.type = type;
  pkt->header.subtype = subtype;
  pkt->header.playerid = local->playerid;
  pkt->header.flags = 0;

  if (type == TYPE_SYSTEM && subtype == 1) {
    pkt->header.sequence = 0;
  }
  else {
    pkt->header.sequence = target->outgoingsequence[type]++;
  }

  if (data && databytes) {
    memcpy(pkt->data, data, databytes);
  }
  pkt->header.checksum = PktGenerateChecksum(pkt);

  MESSAGE* msg = nullptr;
  if (target == local) {
    msg = target->incomingqueue[type].NewNode(STORM_LIST_TAIL, 0, 0);
  }
  else if (type != TYPE_DATAGRAM) {
    msg = target->outgoingqueue[type].NewNode(STORM_LIST_TAIL, 0, 0);
  }

  if (msg) {
    msg->addr = pktaddr;
    msg->data = pkt;
    msg->databytes = databytes + sizeof(HEADER);
    msg->local = true;
    msg->sendtime = PortGetTickCount();
  }

  if (target != local) {
    ConnSendPacket(target, pkt);
    PerfAdd(SNET_PERFID_USERBYTESSENT, databytes);
  }

  if (msg) {
    msg->resendtime = s_spi_outgoingtime;
  }
  else {
    delete pktaddr;
    FREE(pkt);
  }
  return msg;
}

static uint32_t ConnMaintainConnections() {
  uint32_t currtime = PortGetTickCount();
  uint32_t wait = INFINITE;
  CONNREC* local = ConnFindLocal();
  if (!local) return wait;

  CONNREC* next;
  for (CONNREC* conn = s_conn_connlist.Head(); conn; conn = next) {
    next = conn->Next();

    if (conn->playerid == NOPLAYER && currtime - conn->lastreceivetime >= 50000) {
      ConnFree(conn);
      continue;
    }

    if (conn->playerid == NOPLAYER && !conn->establishing) continue;

    if (currtime - conn->lastpingtime >= 20000 && !conn->establishing) {
      conn->lastpingtime = currtime;
      ConnSendMessage(conn, TYPE_SYSTEM, SYS_PING, nullptr, 0);
    }

    for (int32_t type = 0; type < TYPES; type++) {
      if (type == TYPE_DATAGRAM) continue;

      uint16_t lastsequence = conn->incomingsequence[type] - 1;
      for (MESSAGE* message = conn->incomingqueue[type].Head(); message; message = message->Next()) {
        if (message->data->header.sequence - lastsequence > 1) {
          if (message->resendtime && static_cast<int32_t>(s_spi_timetoresend + message->resendtime - currtime) > 0) {
            wait = std::min(wait, s_spi_timetoresend + message->resendtime - currtime);
          }
          else {
            message->resendtime = currtime;
            
            PACKET pkt;
            pkt.header.checksum = 0;
            pkt.header.bytes = sizeof(HEADER);
            pkt.header.sequence = lastsequence + 1;
            pkt.header.acksequence = conn->availablesequence[type];
            pkt.header.type = type;
            pkt.header.subtype = 0;
            pkt.header.playerid = local->playerid;
            pkt.header.flags = MF_RESENDREQUEST;
            pkt.header.checksum = PktGenerateChecksum(&pkt);
            ConnSendPacket(conn, &pkt);
          }
        }
        lastsequence = message->data->header.sequence;
      }
    }

    for (int32_t type = 0; type < TYPES; type++) {
      if (conn->outgoingqueue[type].IsEmpty()) continue;
      if (type == TYPE_TURN || type == TYPE_DATAGRAM) continue;

      MESSAGE* message = conn->outgoingqueue[type].Head();
      uint32_t bandwidth = s_spi_providerptr->caps.bytessec;
      uint32_t maxpacket = s_spi_providerptr->caps.maxmessagesize;
      uint32_t totaldata = message->databytes + maxpacket + 128;
      uint32_t responsetime = 1000 * totaldata / bandwidth + 2 * s_spi_timetoresend + 200;
      if (message->resendtime && static_cast<int32_t>(responsetime + message->resendtime - currtime) > 0) {
        wait = std::min(wait, responsetime + message->resendtime - currtime);
      }
      else {
        message->resendtime = currtime;
        ConnResendMessage(conn, message->data, message->databytes);
      }
    }

    for (int32_t type = 0; type < TYPES; type++) {
      if (type == TYPE_DATAGRAM) continue;
      uint32_t acktime = type == TYPE_TURN ? s_spi_timetoackturn : s_spi_timetorequest;

      if (conn->acksequence[type] == conn->availablesequence[type]) continue;
      if (conn->acktime[type] == 0) continue;

      if (static_cast<int32_t>(acktime + conn->acktime[type] - currtime) > 0) {
        wait = std::min(wait, acktime + conn->acktime[type] - currtime);
      }
      else {
        PACKET pkt;
        pkt.header.checksum = 0;
        pkt.header.bytes = sizeof(HEADER);
        pkt.header.sequence = conn->availablesequence[type];
        pkt.header.acksequence = conn->availablesequence[type];
        pkt.header.type = type;
        pkt.header.subtype = 0;
        pkt.header.playerid = local->playerid;
        pkt.header.flags = MF_ACK;
        pkt.header.checksum = PktGenerateChecksum(&pkt);
        ConnSendPacket(conn, &pkt);
      }
    }
  }
  return wait;
}

static void ConnDestroy() {
  while (CONNREC* curr = s_conn_local.Head()) {
    ConnFree(curr);
  }

  while (CONNREC* curr = s_conn_connlist.Head()) {
    ConnFree(curr);
  }
}

static int STORMAPI SMessageBox(HWND hWnd, const char* lpText, const char* lpCaption, UINT uType) {
  return SDrawMessageBox(lpText, lpCaption, uType);
}

static uint32_t SysBuildPlayerInfo(SYSEVENTDATA_PLAYERINFOPTR data, CONNREC* conn, uint32_t startingturn) {
  data->playerid = conn->playerid;
  data->gameowner = conn->gameowner;
  data->flags = conn->flags;
  data->startingturn = startingturn;
  data->addr = data->addr;

  char* curr = data->namedesc;
  curr += SStrCopy(curr, conn->name, SNETSPI_MAXSTRINGLENGTH) + 1;
  curr += SStrCopy(curr, conn->desc, SNETSPI_MAXSTRINGLENGTH) + 1;
  data->bytes = reinterpret_cast<uint8_t*>(curr) - reinterpret_cast<uint8_t*>(data);
  return data->bytes;
}

static void STORMAPI SysOnCircuitCheck(SYSEVENTPTR event) {
  if (event->databytes == sizeof(uint32_t) && *static_cast<uint32_t*>(event->data) == 1) {
    ConnSendMessage(ConnFindByAddr(event->senderaddr), TYPE_SYSTEM, SYS_CIRCUITCHECKRESPONSE, event->data, 4);
  }
}

static void STORMAPI SysOnDropPlayer(SYSEVENTPTR event) {
  SYSEVENTDATA_DROPPLAYERPTR msg = static_cast<SYSEVENTDATA_DROPPLAYERPTR>(event->data);
  CONNREC* rec = ConnFindByPlayerId(msg->playerid);
  if (rec && rec->playerid != NOPLAYER) {
    rec->flags |= PF_LEAVING;
    rec->finalsequence = msg->finalsequence;
    rec->exitcode = msg->exitcode;
  }
}

static void STORMAPI SysOnNewGameMode(SYSEVENTPTR event) {
  s_game_gamemode = *static_cast<uint32_t*>(event->data);
  s_game_gamepass[0] = (s_game_gamemode & 1) ? s_game_gamepass[0] : '\0';
}

static void STORMAPI SysOnNewGameOwner(SYSEVENTPTR event) {
  for (CONNREC* conn = s_conn_local.Head(); conn; conn = conn->Next()) {
    conn->gameowner = FALSE;
  }
  for (CONNREC* conn = s_conn_connlist.Head(); conn; conn = conn->Next()) {
    conn->gameowner = FALSE;
  }

  CONNREC* conn = ConnFindByPlayerId(*static_cast<uint32_t*>(event->data));
  if (conn) {
    conn->gameowner = TRUE;
  }
}

static void STORMAPI SysOnPing(SYSEVENTPTR event) {
  ConnSendMessage(ConnFindByAddr(event->senderaddr), TYPE_SYSTEM, SYS_PINGRESPONSE, event->data, event->databytes);
}

static void STORMAPI SysOnPingResponse(SYSEVENTPTR event) {
  CONNREC* rec = ConnFindByAddr(event->senderaddr);
  uint32_t ticks = PortGetTickCount();
  if (rec) {
    rec->latency = ticks - rec->lastpingtime;
    rec->peaklatency = std::max(rec->latency, rec->peaklatency);
  }
}

static void SysQueueUserEvent(uint32_t eventid, uint32_t playerid, const void* data, uint32_t databytes) {
  USEREVENT* userevent = s_sys_usereventlist.NewNode(0, 0, 0);
  userevent->event.eventid = eventid;
  userevent->event.playerid = playerid;

  if (data && databytes) {
    userevent->event.data = ALLOC(databytes);
    SMemCopy(userevent->event.data, const_cast<void*>(data), databytes);
    userevent->event.databytes = databytes;
  }

  SCOPE_LOCK(s_sys_usereventlist_critsect);

  s_sys_usereventlist.LinkNode(userevent, STORM_LIST_LINK_BEFORE, nullptr);
}

static void STORMAPI SysOnPlayerJoinAcceptDone(SYSEVENTPTR event) {
  CONNREC* conn = ConnFindLocal();
  if (conn) {
    s_game_playerid = conn->playerid;
  }

  FREEPTRIFUSED(s_game_initdata);
  s_game_initdatabytes = 0;

  if (event->data && event->databytes != 0) {
    s_game_initdata = ALLOC(event->databytes);
    s_game_initdatabytes = event->databytes;
    SMemCopy(s_game_initdata, event->data, s_game_initdatabytes);
    SysQueueUserEvent(1, s_game_playerid, s_game_initdata, s_game_initdatabytes);
  }
}

static void STORMAPI SysOnPlayerJoinAcceptStart(SYSEVENTPTR event) {
  SYSEVENTDATA_PLAYERJOIN_ACCEPTSTARTPTR eventdataptr = static_cast<SYSEVENTDATA_PLAYERJOIN_ACCEPTSTARTPTR>(event->data);
  CONNREC* conn = ConnFindLocal();
  if (conn) {
    ConnAssignPlayerId(conn, eventdataptr->playerid);
    conn->incomingsequence[TYPE_TURN] = eventdataptr->nextturn;
    conn->availablesequence[TYPE_TURN] = eventdataptr->nextturn;
    conn->outgoingsequence[TYPE_TURN] = eventdataptr->nextturn;
    PerfSet(SNET_PERFID_TURN, eventdataptr->nextturn);
  }

  s_game_playersallowed = eventdataptr->playersallowed;
  s_game_gamemode = eventdataptr->gamemode;
  s_game_creationtime = PortGetTickCount() - 1000 * eventdataptr->runningtime;
  
  char* currptr = eventdataptr->namedescpass;
  currptr += SStrCopy(s_game_gamename, currptr, SNETSPI_MAXSTRINGLENGTH) + 1;
  currptr += SStrCopy(s_game_gamedesc, currptr, SNETSPI_MAXSTRINGLENGTH) + 1;
  currptr += SStrCopy(s_game_gamepass, currptr, SNETSPI_MAXSTRINGLENGTH) + 1;
}

static void STORMAPI SysOnPlayerLeave(SYSEVENTPTR event) {
  SYSEVENTDATA_PLAYERLEAVEPTR msg = static_cast<SYSEVENTDATA_PLAYERLEAVEPTR>(event->data);
  CONNREC* rec = ConnFindByAddr(event->senderaddr);
  if (rec && rec->playerid != NOPLAYER) {
    rec->flags |= PF_LEAVING;
    rec->finalsequence = msg->finalsequence;
    rec->exitcode = msg->exitcode;
  }
}

static void STORMAPI SysOnNewLadderId(SYSEVENTPTR event) {
  s_game_ladderid = *static_cast<uint32_t*>(event->data);
}

static void STORMAPI SysOnPlayerJoinReject(SYSEVENTPTR event) {
  // intentionally empty
}

static void SysDestroy() {
  SCOPE_LOCK(s_sys_usereventlist_critsect);
  while (USEREVENT* curr = s_sys_usereventlist.Head()) {
    if (curr->event.data) FREE(curr->event.data);
    s_sys_usereventlist.DeleteNode(curr);
  }
}

static void SysDispatchUserEvents() {
  while(1) {
    USEREVENT* curr;
    {
      SCOPE_LOCK(s_sys_usereventlist_critsect);
      curr = s_sys_usereventlist.Head();
      if (curr) {
        s_sys_usereventlist.UnlinkNode(curr);
      }
    }

    if (!curr) break;

    curr->event.playerid += s_api_playeroffset;
    SEvtDispatch('SNET', 1, curr->event.eventid, &curr->event);

    FREEIFUSED(curr->event.data);
    delete curr;
  }
}

static void RecvProcessExternalMessages() {
  const char* senderpath = nullptr;
  const char* sendername = nullptr;
  const char* message = nullptr;
  while (s_spi->spiReceiveExternalMessage(&senderpath, &sendername, &message)) {
    if (!senderpath || !sendername || !message) break;

    if (!senderpath[0] && !sendername[0]) {
      SysQueueUserEvent(4, -1, message, SStrLen(message) + 1);
    }

    s_spi->spiFreeExternalMessage(senderpath, sendername, message);
    senderpath = nullptr;
    sendername = nullptr;
    message = nullptr;
  }
}

static int32_t SysWaitForMultipleEvents(uint32_t numevents, uint32_t* eventlist, int32_t waitforall, uint32_t timeout) {
  SMemZero(s_sys_event, sizeof(s_sys_event));
  uint32_t starttime = PortGetTickCount();
  bool firstiter = true;
  do {
    if (!firstiter) PortSleep(10);
    firstiter = false;

    RecvProcessExternalMessages();
    //RecvProcessIncomingPackets();
    ConnMaintainConnections();

    uint32_t signalled = 0;
    for (uint32_t i = 0; i < numevents; i++) {
      if (eventlist[i] < SYSMSGS && s_sys_event[eventlist[i]]) {
        signalled++;
      }
    }

    if (signalled >= numevents || signalled && !waitforall) {
      return true;
    }
  } while(timeout == INFINITE || PortGetTickCount() - starttime < timeout);
  return false;
}

static int SpiCheckProviderOrder(PROVIDERINFO* first, PROVIDERINFO* second) {
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

static void SpiDestroy(BOOL clearproviderlist) {
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

static bool SpiDestroyWithError(uint32_t errorcode) {
  SpiDestroy(false);
  SErrSetLastError(errorcode);
  return false;
}

static int32_t SpiInitialize(uint32_t providerid, SNETPROGRAMDATAPTR programdata, SNETPLAYERDATAPTR playerdata, SNETUIDATAPTR interfacedata, SNETVERSIONDATAPTR versiondata, void* recvevent) {
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

static void* SpiLoadCapsSignature(const char* filename) {
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

static BOOL SpiNormalizeDataBlocks(
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

static BOOL SpiSend(uint32_t addresses, SNETADDRPTR* addrlist, void* data, uint32_t databytes) {
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

static void RecvThreadProc() {
  int timeout = -1;
  while (!s_recv_shutdown) {
    BOOL v2 = !WaitForSingleObject(s_recv_event, timeout);  // FIXME: remove Windows dep
    if (s_recv_shutdown) break;

    SCOPE_LOCK(s_api_critsect);

    if (!s_spi) break;

    if (v2) {
      RecvProcessExternalMessages();
      //RecvProcessIncomingPackets();
    }

    timeout = ConnMaintainConnections();
  }
}

static BOOL RecvInitialize(HANDLE* eventptr) {
  if (!s_recv_event) {
    s_recv_event = CreateEvent(nullptr, 0, 0, nullptr);
  }

  if (!s_recv_thread.joinable()) {
    s_recv_shutdown = false;
    s_recv_thread = std::thread(RecvThreadProc);
  }

  if (eventptr) *eventptr = s_recv_event;
  return s_recv_event && s_recv_thread.joinable();
}

// @101
BOOL STORMAPI SNetCreateGame(const char* gamename, const char* gamepassword, const char* gamedescription, uint32_t gamecategorybits, void* initdata, uint32_t initdatabytes, uint32_t maxplayers, const char* playername, const char* playerdescription, uint32_t* playerid) {
  return SNetCreateLadderGame(gamename, gamepassword, gamedescription, gamecategorybits, 0, 0, initdata, initdatabytes, maxplayers, playername, playerdescription, playerid);
}

// @102
BOOL STORMAPI SNetDestroy() {
  {
    SCOPE_LOCK(s_api_critsect);

    if (s_game_playerid != NOPLAYER) {
      SNetLeaveGame(SNET_EXIT_AUTO_SHUTDOWN);
    }
  }

  if (s_recv_thread.joinable()) {
    s_recv_shutdown = true;
    SetEvent(s_recv_event);
    s_recv_thread.join();
    s_recv_shutdown = false;
  }

  SCOPE_LOCK(s_api_critsect);

  SysDestroy();
  SEvtUnregisterType('SNET', 1);
  SEvtUnregisterType('SNET', 2);
  ConnDestroy();
  s_game_playernames.clear();
  SpiDestroy(1);
  if ( s_recv_event )
  {
    CloseHandle(s_recv_event);
    s_recv_event = 0;
  }
  s_game_programid = 0;
  s_game_versionid = 0;
  return TRUE;
}

// @103
BOOL STORMAPI SNetEnumDevices(SNETENUMDEVICESPROC callback) {
  VALIDATEBEGIN;
  VALIDATE(callback);
  VALIDATEEND;

  std::vector<SNETSPI_DEVICELIST> local_device_list;

  {
    SCOPE_LOCK(s_api_critsect);

    if (!s_spi) {
      SErrSetLastError(ERROR_BAD_PROVIDER);
      return FALSE;
    }

    SNETSPI_DEVICELISTPTR listptr;
    if (!s_spi->spiLockDeviceList(&listptr)) {
      return FALSE;
    }

    for (SNETSPI_DEVICELISTPTR p = listptr; p; p = p->next) {
      local_device_list.emplace_back(*p);
      SNETSPI_DEVICELIST& listcopy = local_device_list.back();
      if (p->next) {
        listcopy.next = &listcopy + 1;
      }
    }

    s_spi->spiUnlockDeviceList(listptr);
  }

  for (auto& device : local_device_list) {
    callback(device.deviceid, device.devicename, device.devicedescription);
  }
  return TRUE;
}

// @105
BOOL STORMAPI SNetEnumProviders(SNETCAPSPTR mincaps, SNETENUMPROVIDERSPROC callback) {
  return FALSE;
}

// @106
BOOL STORMAPI SNetDropPlayer(uint32_t playerid, uint32_t exitcode) {
  return FALSE;
}

// @107
BOOL STORMAPI SNetGetGameInfo(uint32_t index, void* buffer, uint32_t buffersize, uint32_t* byteswritten) {
  return FALSE;
}

// @109
BOOL STORMAPI SNetGetNumPlayers(uint32_t* firstplayerid, uint32_t* lastplayerid, uint32_t* activeplayers) {
  SCOPE_LOCK(s_api_critsect);

  if (firstplayerid) *firstplayerid = s_api_playeroffset - 1;
  if (lastplayerid) *lastplayerid = s_api_playeroffset - 1;
  if (activeplayers) *activeplayers = 0;

  if (!s_spi) {
    SErrSetLastError(ERROR_BAD_PROVIDER);
    return FALSE;
  }

  if (s_game_playerid == NOPLAYER) {
    SErrSetLastError(STORM_ERROR_NOT_IN_GAME);
    return FALSE;
  }

  if (firstplayerid) *firstplayerid = s_api_playeroffset + s_game_playerid;
  if (lastplayerid) *lastplayerid = s_api_playeroffset + s_game_playerid;
  if (activeplayers) *activeplayers = 1;

  for (CONNREC* conn = s_conn_connlist.Head(); conn; conn = conn->Next()) {
    if (conn->playerid == NOPLAYER) continue;

    if (firstplayerid) *firstplayerid = std::min(s_api_playeroffset + conn->playerid, *firstplayerid);
    if (lastplayerid) *lastplayerid = std::max(s_api_playeroffset + conn->playerid, *lastplayerid);
    if (activeplayers) (*activeplayers)++;
  }
  return TRUE;
}

// @112
BOOL STORMAPI SNetGetPlayerCaps(uint32_t playerid, SNETCAPSPTR caps) {
  VALIDATEBEGIN;
  VALIDATE(caps && caps->size == sizeof(SNETCAPS));
  VALIDATEEND;

  SCOPE_LOCK(s_api_critsect);

  *caps = { sizeof(SNETCAPS) };

  if (!s_spi || !s_spi_providerptr) {
    SErrSetLastError(ERROR_BAD_PROVIDER);
    return FALSE;
  }

  if (s_game_playerid == NOPLAYER) {
    SErrSetLastError(STORM_ERROR_NOT_IN_GAME);
    return FALSE;
  }

  CONNREC *conn = ConnFindByPlayerId(playerid - s_api_playeroffset);
  if (!conn) {
    SErrSetLastError(STORM_ERROR_INVALID_PLAYER);
    return FALSE;
  }

  *caps = s_spi_providerptr->caps;
  if (conn->latency) {
    caps->latencyms = conn->latency;
  }
  return TRUE;
}

// @113
BOOL STORMAPI SNetGetPlayerName(uint32_t playerid, char* buffer, uint32_t buffersize) {
  STORM_VALIDATE_BEGIN;
  STORM_VALIDATE(buffer);
  STORM_VALIDATE(buffersize != 0);
  STORM_VALIDATE_END;

  SCOPE_LOCK(s_api_critsect);

  buffer[0] = '\0';

  if (!s_spi) {
    SErrSetLastError(ERROR_BAD_PROVIDER);
    return FALSE;
  }

  if (s_game_playerid == NOPLAYER) {
    SErrSetLastError(STORM_ERROR_NOT_IN_GAME);
    return FALSE;
  }

  CONNREC *conn = ConnFindByPlayerId(playerid - s_api_playeroffset);
  if (!conn || (conn->flags & PF_JOINING)) {
    SErrSetLastError(STORM_ERROR_INVALID_PLAYER);
    return FALSE;
  }

  SStrCopy(buffer, conn->name, buffersize);
  return TRUE;
}

// @114
BOOL STORMAPI SNetGetProviderCaps(SNETCAPSPTR caps) {
  VALIDATEBEGIN;
  VALIDATE(caps && caps->size == sizeof(SNETCAPS));
  VALIDATEEND;

  SCOPE_LOCK(s_api_critsect);

  *caps = { sizeof(SNETCAPS) };

  if (!s_spi || !s_spi_providerptr) {
    SErrSetLastError(ERROR_BAD_PROVIDER);
    return FALSE;
  }

  *caps = s_spi_providerptr->caps;
  return TRUE;
}

// @115
BOOL STORMAPI SNetGetTurnsInTransit(uint32_t* turns) {
  VALIDATEBEGIN;
  VALIDATE(turns);
  VALIDATEEND;

  SCOPE_LOCK(s_api_critsect);

  *turns = 0;
  if (!s_spi) {
    SErrSetLastError(ERROR_BAD_PROVIDER);
    return FALSE;
  }

  if (s_game_playerid == NOPLAYER) {
    SErrSetLastError(STORM_ERROR_NOT_IN_GAME);
    return FALSE;
  }

  CONNREC* rec = ConnFindLocal();
  if (!rec) {
    SErrSetLastError(ERROR_NOT_ENOUGH_MEMORY);
    return FALSE;
  }

  *turns = rec->outgoingsequence[TYPE_TURN] - rec->incomingsequence[TYPE_TURN];
  return TRUE;
}

// @116
BOOL STORMAPI SNetInitializeDevice(uint32_t deviceid, SNETPROGRAMDATAPTR programdata, SNETPLAYERDATAPTR playerdata, SNETUIDATAPTR interfacedata, SNETVERSIONDATAPTR versiondata) {
  SCOPE_LOCK(s_api_critsect);

  SNETUIDATA nuidata;
  SNETPROGRAMDATA nprogramdata;
  SNETVERSIONDATA nversiondata;
  SNETPLAYERDATA nplayerdata;

  if (!s_spi) {
    SErrSetLastError(ERROR_BAD_PROVIDER);
    return FALSE;
  }

  if (!SpiNormalizeDataBlocks(programdata, playerdata, interfacedata, versiondata, &nprogramdata, &nplayerdata, &nuidata, &nversiondata)) {
    SErrSetLastError(ERROR_INVALID_PARAMETER);
    return FALSE;
  }

  if (!s_spi->spiInitializeDevice(deviceid, &nprogramdata, &nplayerdata, &nuidata, &nversiondata)) {
    return FALSE;
  }
  return TRUE;
}

// @117
BOOL STORMAPI SNetInitializeProvider(uint32_t providerid, SNETPROGRAMDATAPTR programdata, SNETPLAYERDATAPTR playerdata, SNETUIDATAPTR interfacedata, SNETVERSIONDATAPTR versiondata) {
  SCOPE_LOCK(s_api_critsect);

  SNETUIDATA nuidata;
  SNETPROGRAMDATA nprogramdata;
  SNETVERSIONDATA nversiondata;
  SNETPLAYERDATA nplayerdata;

  if (!SpiNormalizeDataBlocks(programdata, playerdata, interfacedata, versiondata, &nprogramdata, &nplayerdata, &nuidata, &nversiondata)) {
    SErrSetLastError(ERROR_INVALID_PARAMETER);
    return FALSE;
  }

  SEvtUnregisterType('SNET', 1);
  SEvtUnregisterType('SNET', 2);

  SEvtRegisterHandler('SNET', 2, 2, 0, (SEVTHANDLER)SysOnCircuitCheck);
  SEvtRegisterHandler('SNET', 2, 12, 0, (SEVTHANDLER)SysOnDropPlayer);
  SEvtRegisterHandler('SNET', 2, 14, 0, (SEVTHANDLER)SysOnNewGameMode);
  SEvtRegisterHandler('SNET', 2, 13, 0, (SEVTHANDLER)SysOnNewGameOwner);
  SEvtRegisterHandler('SNET', 2, 15, 0, (SEVTHANDLER)SysOnNewLadderId);
  SEvtRegisterHandler('SNET', 2, 4, 0, (SEVTHANDLER)SysOnPing);
  SEvtRegisterHandler('SNET', 2, 5, 0, (SEVTHANDLER)SysOnPingResponse);
  //SEvtRegisterHandler('SNET', 2, 6, 0, (SEVTHANDLER)SysOnPlayerInfo);
  //SEvtRegisterHandler('SNET', 2, 7, 0, (SEVTHANDLER)SysOnPlayerJoin);
  SEvtRegisterHandler('SNET', 2, 8, 0, (SEVTHANDLER)SysOnPlayerJoinAcceptStart);
  SEvtRegisterHandler('SNET', 2, 9, 0, (SEVTHANDLER)SysOnPlayerJoinAcceptDone);
  SEvtRegisterHandler('SNET', 2, 10, 0, (SEVTHANDLER)SysOnPlayerJoinReject);
  SEvtRegisterHandler('SNET', 2, 11, 0, (SEVTHANDLER)SysOnPlayerLeave);

  HANDLE hEvent = NULL;
  if (!RecvInitialize(&hEvent)) {
    SErrSetLastError(ERROR_MAX_THRDS_REACHED);
    return FALSE;
  }

  if (!SpiInitialize(providerid, &nprogramdata, &nplayerdata, &nuidata, &nversiondata, hEvent)) {
    return FALSE;
  }

  s_game_optcategorybits = nprogramdata.optcategorybits;
  s_game_programid = nprogramdata.programid;
  s_game_versionid = nprogramdata.versionid;
  return TRUE;
}

// @118
BOOL STORMAPI SNetJoinGame(uint32_t gameid, const char* gamename, const char* gamepassword, const char* playername, const char* playerdescription, uint32_t* playerid) {
  return FALSE;
}

// @119
BOOL STORMAPI SNetLeaveGame(uint32_t exitcode) {
  return FALSE;
}

// @120
BOOL STORMAPI SNetPerformUpgrade(uint32_t* upgradestatus) {
  return FALSE;
}

// @121
BOOL STORMAPI SNetReceiveMessage(uint32_t* senderplayerid, void** data, uint32_t* databytes) {
  return FALSE;
}

// @122
BOOL STORMAPI SNetReceiveTurns(uint32_t firstplayerid, uint32_t arraysize, void** arraydata, uint32_t* arraydatabytes, uint32_t* arrayplayerstatus) {
  return FALSE;
}

// @123
BOOL STORMAPI SNetRegisterEventHandler(uint32_t eventid, SNETEVENTPROC callback) {
  VALIDATEBEGIN;
  VALIDATE(callback);
  VALIDATEEND;

  BOOL result;
  {
    SCOPE_LOCK(s_api_critsect);
    result = SEvtRegisterHandler('SNET', 1, eventid, 0, reinterpret_cast<SEVTHANDLER>(callback));
  }

  if (!result) {
    SErrSetLastError(ERROR_NOT_ENOUGH_MEMORY);
  }
  return result;
}

// @124
BOOL STORMAPI SNetResetLatencyMeasurements() {
  SCOPE_LOCK(s_api_critsect);

  for (CONNREC* conn = s_conn_connlist.Head(); conn; conn = conn->Next()) {
    conn->latency = 0;
    conn->peaklatency = 0;
    conn->lastpingtime = 0;
  }
  return TRUE;
}

// @125
BOOL STORMAPI SNetSelectGame(uint32_t flags, SNETPROGRAMDATAPTR programdata, SNETPLAYERDATAPTR playerdata, SNETUIDATAPTR interfacedata, SNETVERSIONDATAPTR versiondata, uint32_t* playerid) {
  SNETUIDATA nuidata;
  SNETPROGRAMDATA nprogramdata;
  SNETVERSIONDATA nversiondata;
  SNETPLAYERDATA nplayerdata;

  decltype(SNETSPI::spiSelectGame) fn_spiSelectGame;

  {
    SCOPE_LOCK(s_api_critsect);
    if (playerid) *playerid = 0;

    if (!s_spi) {
      SErrSetLastError(ERROR_BAD_PROVIDER);
      return FALSE;
    }

    if (!SpiNormalizeDataBlocks(programdata, playerdata, interfacedata, versiondata, &nprogramdata, &nplayerdata, &nuidata, &nversiondata)) {
      SErrSetLastError(ERROR_INVALID_PARAMETER);
      return FALSE;
    }

    fn_spiSelectGame = s_spi->spiSelectGame;
  }
  return fn_spiSelectGame(flags, &nprogramdata, &nplayerdata, &nuidata, &nversiondata, playerid);
}

// @127
BOOL STORMAPI SNetSendMessage(uint32_t targetplayerid, void* data, uint32_t databytes) {
  VALIDATEBEGIN;
  VALIDATE(data);
  VALIDATE(databytes);
  VALIDATEEND;

  {
    SCOPE_LOCK(s_api_critsect);

    if (!s_spi) {
      SErrSetLastError(ERROR_BAD_PROVIDER);
      return FALSE;
    }

    if (s_game_playerid == NOPLAYER) {
      SErrSetLastError(STORM_ERROR_NOT_IN_GAME);
      return FALSE;
    }

    if (targetplayerid != SNET_BROADCASTNONLOCALPLAYERID && targetplayerid != SNET_BROADCASTPLAYERID) {
      targetplayerid -= s_api_playeroffset;
    }

    if (targetplayerid != SNET_BROADCASTNONLOCALPLAYERID && targetplayerid != SNET_BROADCASTPLAYERID) {
      CONNREC* target = ConnFindByPlayerId(targetplayerid);
      if (!target) {
        SErrSetLastError(STORM_ERROR_INVALID_PLAYER);
        return FALSE;
      }
      ConnSendMessage(target, TYPE_MESSAGE, 0, data, databytes);
    }
    else {
      for (CONNREC* conn = s_conn_connlist.Head(); conn; conn = conn->Next()) {
        if (conn->playerid != NOPLAYER) {
          ConnSendMessage(conn, TYPE_MESSAGE, 0, data, databytes);
        }
      }

      if (targetplayerid == SNET_BROADCASTPLAYERID) {
        CONNREC* local = ConnFindLocal();
        if (local) {
          ConnSendMessage(local, TYPE_MESSAGE, 0, data, databytes);
        }
      }
    }
    PerfIncrement(SNET_PERFID_MSGSENT);
  }
  SetEvent(s_recv_event);
  return TRUE;
}

// @128
BOOL STORMAPI SNetSendTurn(void* data, uint32_t databytes) {
  VALIDATEBEGIN;
  VALIDATE(data);
  VALIDATE(databytes);
  VALIDATEEND;

  {
    SCOPE_LOCK(s_api_critsect);

    if (!s_spi) {
      SErrSetLastError(ERROR_BAD_PROVIDER);
      return FALSE;
    }

    if (s_game_playerid == NOPLAYER) {
      SErrSetLastError(STORM_ERROR_NOT_IN_GAME);
      return FALSE;
    }

    CONNREC* local = ConnFindLocal();
    if (!local) {
      SErrSetLastError(STORM_ERROR_NOT_IN_GAME);
      return FALSE;
    }

    for (CONNREC* conn = s_conn_connlist.Head(); conn; conn = conn->Next()) {
      if (conn->playerid != NOPLAYER && conn->outgoingsequence[TYPE_TURN] == local->outgoingsequence[TYPE_TURN]) {
        ConnSendMessage(conn, TYPE_TURN, 0, data, databytes);
      }
    }
    ConnSendMessage(local, TYPE_TURN, 0, data, databytes);
    PerfIncrement(SNET_PERFID_TURNSSENT);
  }
  SetEvent(s_recv_event);
  return TRUE;
}

// @129
BOOL STORMAPI SNetSetBasePlayer(int playerid) {
  SCOPE_LOCK(s_api_critsect);
  s_api_playeroffset = playerid;
  return TRUE;
}

// @130
BOOL STORMAPI SNetSetGameMode(uint32_t modeflags, int32_t makepublic) {
  SCOPE_LOCK(s_api_critsect);

  if (!s_spi) {
    SErrSetLastError(ERROR_INVALID_PARAMETER);
    return FALSE;
  }

  if (s_game_playerid == NOPLAYER) {
    SErrSetLastError(STORM_ERROR_NOT_IN_GAME);
    return FALSE;
  }

  CONNREC* local = ConnFindLocal();
  if (!local || !local->gameowner) {
    SErrSetLastError(ERROR_NOT_OWNER);
    return FALSE;
  }

  if (makepublic) {
    modeflags &= ~SNET_GM_PRIVATE;
    s_game_gamepass[0] = '\0';
  }

  if (modeflags == s_game_gamemode) {
    return TRUE;
  }

  s_game_gamemode = modeflags;
  for (CONNREC* checkconn = s_conn_connlist.Head(); checkconn; checkconn = checkconn->Next()) {
    if (checkconn->playerid != NOPLAYER) {
      ConnSendMessage(checkconn, TYPE_SYSTEM, SYS_GAMEMODE, &s_game_gamemode, sizeof(s_game_gamemode));
    }
  }

  if (GameStartAdvertising()) {
    return TRUE;
  }

  return FALSE;
}

// @131
BOOL STORMAPI SNetUnregisterEventHandler(uint32_t eventid, SNETEVENTPROC callback) {
  VALIDATEBEGIN;
  VALIDATE(callback);
  VALIDATEEND;

  BOOL result;
  {
    SCOPE_LOCK(s_api_critsect);
    result = SEvtUnregisterHandler('SNET', 1, eventid, (SEVTHANDLER)callback);
  }

  if (!result) {
    SErrSetLastError(STORM_ERROR_NOT_REGISTERED);
  }
  return result;
}

// @133
BOOL STORMAPI SNetEnumGamesEx(uint32_t categorybits, uint32_t categorymask, SNETENUMGAMESEXPROC callback, uint32_t* hintnextcall) {
  return FALSE;
}

// @134
BOOL STORMAPI SNetSendServerChatCommand(const char* command) {
  VALIDATEBEGIN;
  VALIDATE(command);
  VALIDATE(*command);
  VALIDATEEND;

  SCOPE_LOCK(s_api_critsect);

  if (!s_spi) {
    SErrSetLastError(ERROR_BAD_PROVIDER);
    return FALSE;
  }

  if (s_game_playerid == NOPLAYER) {
    SErrSetLastError(STORM_ERROR_NOT_IN_GAME);
    return FALSE;
  }

  CONNREC* conn = ConnFindLocal();
  if (!conn) {
    SErrSetLastError(STORM_ERROR_NOT_IN_GAME);
    return FALSE;
  }

  char path[144];
  std::snprintf(path, sizeof(path), "\\\\.\\game\\%s", s_game_gamename);
  return s_spi->spiSendExternalMessage(path, conn->name, "", "", command);
}

// @137
BOOL STORMAPI SNetDisconnectAll(uint32_t exitcode) {
  SCOPE_LOCK(s_api_critsect);

  if (!s_spi) {
    SErrSetLastError(ERROR_BAD_PROVIDER);
    return FALSE;
  }

  if (s_game_playerid == NOPLAYER) {
    SErrSetLastError(STORM_ERROR_NOT_IN_GAME);
    return FALSE;
  }

  for (CONNREC *conn = s_conn_connlist.Head(); conn; conn = conn->Next()) {
    conn->flags |= PF_LEAVING;
    conn->finalsequence = conn->incomingsequence[TYPE_TURN];
    conn->exitcode = exitcode;
  }
  return TRUE;
}

// @138
BOOL STORMAPI SNetCreateLadderGame(const char* gamename, const char* gamepassword, const char* gamedescription, uint32_t gamecategorybits, uint32_t ladderid, uint32_t gamemode, void* initdata, uint32_t initdatabytes, uint32_t maxplayers, const char* playername, const char* playerdescription, uint32_t* playerid) {
  return FALSE;
}

// @139
BOOL STORMAPI SNetReportGameResult(uint32_t firstplayerid, uint32_t arraysize, uint32_t* resultarray, const char* textgameresult, const char* textplayerresult) {
  VALIDATEBEGIN;
  VALIDATE(arraysize);
  VALIDATE(resultarray);
  VALIDATEEND;

  SCOPE_LOCK(s_api_critsect);

  if (!s_spi) {
    SErrSetLastError(ERROR_BAD_PROVIDER);
    return FALSE;
  }

  const char** playernames = static_cast<const char**>(ALLOCZERO(sizeof(char*) * arraysize));
  int i = 0;
  for (unsigned id = firstplayerid; id < firstplayerid + arraysize; id++) {
    if (id < s_game_playernames.size()) {
      playernames[i] = s_game_playernames[id].name;
    }
    i++;
  }

  if (s_spi->spiReportGameResult) {
    s_spi->spiReportGameResult(s_game_ladderid, arraysize, playernames, resultarray, textgameresult, textplayerresult);
  }
  FREE(playernames);
  return TRUE;
}

// @140
BOOL STORMAPI SNetCheckDataFile(const char* filename, const void* data, uint32_t bytes, uint32_t* extendedresult) {
  if (extendedresult) *extendedresult = 0;

  VALIDATEBEGIN;
  VALIDATE(filename);
  VALIDATE(*filename);
  VALIDATE(data);
  VALIDATE(extendedresult);
  VALIDATEEND;

  SCOPE_LOCK(s_api_critsect);

  if (!s_spi) {
    SErrSetLastError(ERROR_BAD_PROVIDER);
    return FALSE;
  }

  if (s_spi->spiCheckDataFile) {
    s_spi->spiCheckDataFile(filename, data, bytes, extendedresult);
  }
  return *extendedresult != 0;
}

// @141
BOOL STORMAPI SNetSendLeagueCommand(const char* cmd, SNETLEAGUECMDRESULTPROC callback) {
  VALIDATEBEGIN;
  VALIDATE(cmd);
  VALIDATEEND;

  SCOPE_LOCK(s_api_critsect);

  if (cmd[0] != '/') {
    SErrSetLastError(ERROR_INVALID_PARAMETER);
    return FALSE;
  }

  if (!s_spi || !s_spi->spiSendLeagueCommand) {
    SErrSetLastError(ERROR_BAD_PROVIDER);
    return FALSE;
  }

  if (!s_spi->spiSendLeagueCommand(cmd, callback)) {
    SErrSetLastError(ERROR_GEN_FAILURE);
    return FALSE;
  }

  return TRUE;
}

// @142
int STORMAPI SNetSendReplayPath(const char* replaypath, uint32_t gameid, const char* textgameresult) {
  VALIDATEBEGIN;
  VALIDATE(replaypath);
  VALIDATEEND;

  SCOPE_LOCK(s_api_critsect);

  int result = 0;
  if (s_spi && s_spi->spiSendReplayPath) {
    result = s_spi->spiSendReplayPath(replaypath, gameid, textgameresult);
    if (!result) {
      SErrSetLastError(ERROR_GEN_FAILURE);
    }
  }
  else {
    SErrSetLastError(ERROR_BAD_PROVIDER);
  }
  return result;
}

// @143
int STORMAPI SNetGetLeagueId(uint32_t* leagueID) {
  VALIDATEBEGIN;
  VALIDATE(leagueID);
  VALIDATEEND;

  SCOPE_LOCK(s_api_critsect);

  int result = 0;
  if (s_spi && s_spi->spiGetLeagueId) {
    result = s_spi->spiGetLeagueId(leagueID);
    if (!result) {
      SErrSetLastError(ERROR_GEN_FAILURE);
    }
  }
  else {
    SErrSetLastError(ERROR_BAD_PROVIDER);
  }
  return result;
}

// @144
BOOL STORMAPI SNetGetPlayerNames(char** names) {
  for (int i = 0; i < 8; i++) {
    if (s_game_playernames.size() > i) {
      names[i] = s_game_playernames[i].name;
    }
  }
  return TRUE;
}

// @145
int STORMAPI SNetLeagueLogout(char* bnetName) {
  VALIDATEBEGIN;
  VALIDATE(bnetName);
  VALIDATEEND;

  SCOPE_LOCK(s_api_critsect);

  if (!s_spi || !s_spi->spiLeagueLogout) {
    SErrSetLastError(ERROR_BAD_PROVIDER);
    return FALSE;
  }

  if (!s_spi->spiLeagueLogout(bnetName)) {
    SErrSetLastError(ERROR_GEN_FAILURE);
    return FALSE;
  }
  return TRUE;
}

// @146
int STORMAPI SNetGetReplyName(char* pszReplyName, size_t nameSize) {
  VALIDATEBEGIN;
  VALIDATE(pszReplyName);
  VALIDATE(nameSize != 0);
  VALIDATEEND;

  SCOPE_LOCK(s_api_critsect);

  int result = 0;
  if (s_spi && s_spi->spiGetReplyName) {
    if (!s_spi->spiGetReplyName(pszReplyName, nameSize)) {
      SErrSetLastError(ERROR_GEN_FAILURE);
    }
  }
  else {
    SErrSetLastError(ERROR_BAD_PROVIDER);
  }
  return TRUE;
}

// @147
// Returns 4 byte protocol identifier of current protocol, only used for debugging, can be removed.
uint32_t STORMAPI SNetGetCurrentProviderID() {
  return s_spi_providerptr ? s_spi_providerptr->id : 0;
}

// @148
void SNetSetCodeSignVerifcationFunction(CODEVERIFYPROC verifyfn) {
  s_CodeSignFunc = verifyfn;
}
