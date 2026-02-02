#include <storm/Memory.hpp>
#include <storm/Event.hpp>
#include "SErr.h"
#include "SNet.h"
#include "SMem.h"
#include "SDraw.h"

#include <SDL.h>

#include <atomic>
#include <chrono>
#include <filesystem>
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
  BYTE data[];
} *PACKETPTR;

struct MESSAGE : TSLinkedNode<MESSAGE>
{
  SNETADDRPTR addr;
  PACKETPTR data;
  DWORD databytes;
  BOOL local;
  DWORD sendtime;
  DWORD resendtime;
};

struct CONNREC : TSLinkedNode<CONNREC>
{
  char name[128];
  char desc[128];
  SNETADDR addr;
  DWORD flags;
  DWORD lastreceivetime;
  DWORD lastrequesttime;
  DWORD lastpingtime;
  DWORD latency;
  DWORD peaklatency;
  STORM_LIST(MESSAGE) outgoingqueue[4];
  STORM_LIST(MESSAGE) incomingqueue[4];
  STORM_LIST(MESSAGE) processing[4];
  STORM_LIST(MESSAGE) oldturns;
  WORD outgoingsequence[4];
  WORD incomingsequence[4];
  WORD lastprocessedturn;
  WORD availablesequence[4];
  WORD acksequence[4];
  DWORD acktime[4];
  BOOL gameowner;
  BOOL establishing;
  DWORD exitcode;
  uint32_t finalsequence;
  uint16_t unk;
  uint8_t playerid;
  uint8_t oldplayerid;
};

struct PROVIDERINFO {
  char     filename[MAX_PATH];
  uint32_t index;
  uint32_t id;
  uint32_t field_20C;
  uint32_t field_210;
  char     desc[SNETSPI_MAXSTRINGLENGTH];
  char     req[SNETSPI_MAXSTRINGLENGTH];
  SNETCAPS caps;
};

struct _PLAYERNAME {
  char name[128];
};

struct USEREVENT : TSLinkedNode<USEREVENT> {
  SNETEVENT event;
} *USEREVENTPTR;

typedef struct _SYSEVENT {
  SNETADDRPTR senderaddr;
  void*       data;
  uint32_t    databytes;
  uint8_t     senderplayerid;
  uint8_t     eventid;
} SYSEVENT, *SYSEVENTPTR;


static std::recursive_mutex s_api_critsect;

static SNETSPIPTR s_spi;
static std::vector<PROVIDERINFO> s_spi_providerlist;
static PROVIDERINFO* s_spi_providerptr;
static int s_api_playeroffset;

static CODEVERIFYPROC s_CodeSignFunc;

static char s_game_playerid = SNET_INVALIDPLAYERID;
static std::thread s_recv_thread;
static std::atomic_bool s_recv_shutdown;
static HANDLE s_recv_event;  // FIXME: move off Windows

static STORM_LIST(CONNREC) s_conn_local;
static STORM_LIST(CONNREC) s_conn_connlist;

static std::vector<_PLAYERNAME> s_game_playernames;
static uint32_t s_game_categorybits;
static uint32_t s_game_creationtime;
static uint32_t s_game_initdata;
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


static uint32_t PortGetTickCount() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
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

static void ConnAssignPlayerId(CONNREC* conn, BYTE playerid) {
  conn->oldplayerid = SNET_INVALIDPLAYERID;
  conn->playerid = playerid;
  
  for (CONNREC* curr = s_conn_local.Head(); curr; curr = curr->Next()) {
    if (curr->oldplayerid == playerid) {
      curr->oldplayerid = SNET_INVALIDPLAYERID;
    }
  }
  
  for (CONNREC* curr = s_conn_connlist.Head(); curr; curr = curr->Next()) {
    if (curr->oldplayerid == playerid) {
      curr->oldplayerid = SNET_INVALIDPLAYERID;
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
    if (curr->oldplayerid == playerid && curr->playerid == SNET_INVALIDPLAYERID) {
      return curr;
    }
  }

  for (CONNREC* curr = s_conn_connlist.Head(); curr; curr = curr->Next()) {
    if (curr->oldplayerid == playerid && curr->playerid == SNET_INVALIDPLAYERID) {
      return curr;
    }
  }
  return nullptr;
}

static CONNREC* ConnAddRec(STORM_LIST(CONNREC)* list, SNETADDRPTR addr) {
  CONNREC *newconn = list->NewNode(STORM_LIST_TAIL, 0, 0);
  newconn->addr = *addr;
  newconn->playerid = SNET_INVALIDPLAYERID;
  newconn->oldplayerid = SNET_INVALIDPLAYERID;
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

static void ConnFree(CONNREC *conn) {
  for (int i = 0; i < 4; i++) {
    ConnDestroyQueue(&conn->outgoingqueue[i]);
    ConnDestroyQueue(&conn->incomingqueue[i]);
    ConnDestroyQueue(&conn->processing[i]);
  }
  ConnDestroyQueue(&conn->oldturns);
  delete conn;
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

static void STORMAPI SysOnCircuitCheck(SYSEVENTPTR event) {
  if (event->databytes == 4 && *static_cast<uint32_t*>(event->data) == 1) {
    //ConnSendMessage(ConnFindByAddr(event->senderaddr), 0, 3, event->data, 4);
  }
}

static void STORMAPI SysOnNewGameMode(SYSEVENTPTR event) {
  s_game_gamemode = *static_cast<uint32_t*>(event->data);
  s_game_gamepass[0] = (s_game_gamemode & 1) ? s_game_gamepass[0] : '\0';
}

static void STORMAPI SysOnPing(SYSEVENTPTR event) {
  //ConnSendMessage(ConnFindByAddr(event->senderaddr), 0, 5, event->data, event->databytes);
}

static void STORMAPI SysOnPingResponse(SYSEVENTPTR event) {
  CONNREC* rec = ConnFindByAddr(event->senderaddr);
  uint32_t ticks = PortGetTickCount();
  if (rec) {
    rec->latency = ticks - rec->lastpingtime;
    rec->peaklatency = std::max(rec->latency, rec->peaklatency);
  }
}

struct MSG_PLYR_LEAVE {
  uint16_t field_0;
  uint32_t seq;
};

static void STORMAPI SysOnPlayerLeave(SYSEVENTPTR event) {
  MSG_PLYR_LEAVE* msg = static_cast<MSG_PLYR_LEAVE*>(event->data);
  CONNREC* rec = ConnFindByAddr(event->senderaddr);
  if (rec && rec->playerid != SNET_INVALIDPLAYERID) {
    rec->flags |= 8;
    rec->unk = msg->field_0;
    rec->finalsequence = msg->seq;
  }
}

static void STORMAPI SysOnNewLadderId(SYSEVENTPTR event) {
  s_game_ladderid = *static_cast<uint32_t*>(event->data);
}

static void STORMAPI SysOnPlayerJoinReject(SYSEVENTPTR event) {
  // intentionally empty
}

int SpiCheckProviderOrder(PROVIDERINFO* first, PROVIDERINFO* second) {
  static const DWORD baseorder[] = {'BNET', 'IPXN', 'IPXW', 'MODM', 'SCBL', 'MSDP'};
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

static void RecvThreadProc() {
  int timeout = -1;
  while (!s_recv_shutdown) {
    BOOL v2 = !WaitForSingleObject(s_recv_event, timeout);  // FIXME: remove Windows dep
    if (s_recv_shutdown) break;

    SCOPE_LOCK(s_api_critsect);

    if (!s_spi) break;

    if (v2) {
      //RecvProcessExternalMessages();
      //RecvProcessIncomingPackets();
    }

    //timeout = ConnMaintainConnections();
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

    if (s_game_playerid != SNET_INVALIDPLAYERID) {
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

  //SysDestroy();
  SEvtUnregisterType('SNET', 1);
  SEvtUnregisterType('SNET', 2);
  ConnDestroy();
  s_game_playernames.clear();
  //SpiDestroy(1);
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
  return FALSE;
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

  if (s_game_playerid == SNET_INVALIDPLAYERID) {
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

  if (s_game_playerid == SNET_INVALIDPLAYERID) {
    SErrSetLastError(STORM_ERROR_NOT_IN_GAME);
    return FALSE;
  }

  CONNREC *conn = ConnFindByPlayerId(playerid - s_api_playeroffset);
  if (!conn || (conn->flags & 4)) {
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

  if (s_game_playerid == SNET_INVALIDPLAYERID) {
    SErrSetLastError(STORM_ERROR_NOT_IN_GAME);
    return FALSE;
  }

  CONNREC* rec = ConnFindLocal();
  if (!rec) {
    SErrSetLastError(ERROR_NOT_ENOUGH_MEMORY);
    return FALSE;
  }

  *turns = rec->outgoingsequence[2] - rec->incomingsequence[2];
  return TRUE;
}

// @116
BOOL STORMAPI SNetInitializeDevice(uint32_t deviceid, SNETPROGRAMDATAPTR programdata, SNETPLAYERDATAPTR playerdata, SNETUIDATAPTR interfacedata, SNETVERSIONDATAPTR versiondata) {
  return FALSE;
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
  //SEvtRegisterHandler('SNET', 2, 12, 0, (SEVTHANDLER)SysOnDropPlayer);
  SEvtRegisterHandler('SNET', 2, 14, 0, (SEVTHANDLER)SysOnNewGameMode);
  //SEvtRegisterHandler('SNET', 2, 13, 0, (SEVTHANDLER)SysOnNewGameOwner);
  SEvtRegisterHandler('SNET', 2, 15, 0, (SEVTHANDLER)SysOnNewLadderId);
  SEvtRegisterHandler('SNET', 2, 4, 0, (SEVTHANDLER)SysOnPing);
  SEvtRegisterHandler('SNET', 2, 5, 0, (SEVTHANDLER)SysOnPingResponse);
  //SEvtRegisterHandler('SNET', 2, 6, 0, (SEVTHANDLER)SysOnPlayerInfo);
  //SEvtRegisterHandler('SNET', 2, 7, 0, (SEVTHANDLER)SysOnPlayerJoin);
  //SEvtRegisterHandler('SNET', 2, 8, 0, (SEVTHANDLER)SysOnPlayerJoinAcceptStart);
  //SEvtRegisterHandler('SNET', 2, 9, 0, (SEVTHANDLER)SysOnPlayerJoinAcceptDone);
  SEvtRegisterHandler('SNET', 2, 10, 0, (SEVTHANDLER)SysOnPlayerJoinReject);
  SEvtRegisterHandler('SNET', 2, 11, 0, (SEVTHANDLER)SysOnPlayerLeave);

  HANDLE hEvent = NULL;
  if (!RecvInitialize(&hEvent)) {
    SErrSetLastError(ERROR_MAX_THRDS_REACHED);
    return FALSE;
  }

  /*if (!SpiInitialize(providerid, &nprogramdata, &nplayerdata, &nuidata, &nversiondata, hEvent))*/ {
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
  return FALSE;
}

// @127
BOOL STORMAPI SNetSendMessage(uint32_t targetplayerid, void* data, uint32_t databytes) {
  return FALSE;
}

// @128
BOOL STORMAPI SNetSendTurn(void* data, uint32_t databytes) {
  return FALSE;
}

// @129
BOOL STORMAPI SNetSetBasePlayer(int playerid) {
  SCOPE_LOCK(s_api_critsect);
  s_api_playeroffset = playerid;
  return TRUE;
}

// @130
BOOL STORMAPI SNetSetGameMode(uint32_t modeflags) {
  return FALSE;
}

// @133
BOOL STORMAPI SNetEnumGamesEx(uint32_t categorybits, uint32_t categorymask, SNETENUMGAMESEXPROC callback, uint32_t* hintnextcall) {
  return FALSE;
}

// @134
BOOL STORMAPI SNetSendServerChatCommand(const char* command) {
  return FALSE;
}

// @137
BOOL STORMAPI SNetDisconnectAll(uint32_t flags) {
  SCOPE_LOCK(s_api_critsect);

  if (!s_spi) {
    SErrSetLastError(ERROR_BAD_PROVIDER);
    return FALSE;
  }

  if (s_game_playerid == SNET_INVALIDPLAYERID) {
    SErrSetLastError(STORM_ERROR_NOT_IN_GAME);
    return FALSE;
  }

  for (CONNREC *conn = s_conn_connlist.Head(); conn; conn = conn->Next()) {
    conn->flags |= 8;
    conn->unk = conn->incomingsequence[2];
    conn->finalsequence = flags;
  }
  return TRUE;
}

// @138
BOOL STORMAPI SNetCreateLadderGame(const char* gamename, const char* gamepassword, const char* gamedescription, uint32_t gamecategorybits, uint32_t ladderid, uint32_t gamemode, void* initdata, uint32_t initdatabytes, uint32_t maxplayers, const char* playername, const char* playerdescription, uint32_t* playerid) {
  return FALSE;
}

// @139
BOOL STORMAPI SNetReportGameResult(unsigned firstplayerid, int arraysize, int* resultarray, const char* textgameresult, const char* textplayerresult) {
  return TRUE;
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
