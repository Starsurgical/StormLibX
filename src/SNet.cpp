#include <storm/Memory.hpp>
#include <storm/Event.hpp>
#include "SErr.h"
#include "SNet.h"
#include "SDraw.h"

#include <SDL.h>

#include <atomic>
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
    std::uint32_t entrysize;
    std::uint32_t providerid;
    char description[]; // null terminated string of any length
    SNETCAPS caps;
  }[]; // repeats for the number of providers
*/

struct SNETADDR {
  uint8_t address[16];
};

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
  char field_212;
  char field_213;
};

struct PROVIDERINFO {
  char     filename[MAX_PATH];
  std::uint32_t    index;
  std::uint32_t    id;
  char     desc[SNETSPI_MAXSTRINGLENGTH];
  char     req[SNETSPI_MAXSTRINGLENGTH];
  SNETCAPS caps;
};

struct _PLAYERNAME {
  char name[128];
};

static std::vector<_PLAYERNAME> s_game_playernames;

static void GameSetPlayerName(unsigned int id, const char *name) {
  STORM_VALIDATE_BEGIN;
  STORM_VALIDATE(id < 256);
  STORM_VALIDATE_END;

  if (id + 1 > s_game_playernames.size()) {
    s_game_playernames.resize(id + 1, {});
  }

  SStrCopy(s_game_playernames[id].name, name, sizeof(_PLAYERNAME::name));
}

namespace {
  std::recursive_mutex s_api_critsect;

  SNETSPIPTR s_spi;
  std::vector<PROVIDERINFO> s_spi_providerlist;
  PROVIDERINFO* s_spi_providerptr;

  CODEVERIFYPROC s_verify_fn;

  char s_game_playerid = -1;
  std::thread s_recv_thread;
  std::atomic_bool s_recv_shutdown;
  HANDLE s_recv_event;  // FIXME: move off Windows

  uint32_t s_game_optcategorybits;
  uint32_t s_game_programid;
  uint32_t s_game_versionid;

  STORM_LIST(CONNREC) s_conn_connlist;


  int STORMAPI SMessageBox(HWND hWnd, const char* lpText, const char* lpCaption, UINT uType) {
    return SDrawMessageBox(lpText, lpCaption, uType);
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

  void RecvThreadProc() {
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

  BOOL RecvInitialize(HANDLE* eventptr) {
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
}

// @101
BOOL STORMAPI SNetCreateGame(const char* gamename, const char* gamepassword, const char* gamedescription, std::uint32_t gamecategorybits, void* initdata, std::uint32_t initdatabytes, std::uint32_t maxplayers, const char* playername, const char* playerdescription, std::uint32_t* playerid) {
  return SNetCreateLadderGame(gamename, gamepassword, gamedescription, gamecategorybits, 0, 0, initdata, initdatabytes, maxplayers, playername, playerdescription, playerid);
}

// @102
BOOL STORMAPI SNetDestroy() {
  {
    SCOPE_LOCK(s_api_critsect);

    if (s_game_playerid != -1) {
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
  /*ConnDestroy();
  dword_1506B850 = 0;
  SpiDestroy(1);*/
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
BOOL STORMAPI SNetDropPlayer(std::uint32_t playerid, std::uint32_t exitcode) {
  return FALSE;
}

// @107
BOOL STORMAPI SNetGetGameInfo(std::uint32_t index, void* buffer, std::uint32_t buffersize, std::uint32_t* byteswritten) {
  return FALSE;
}

// @109
BOOL STORMAPI SNetGetNumPlayers(std::uint32_t* firstplayerid, std::uint32_t* lastplayerid, std::uint32_t* activeplayers) {
  return FALSE;
}

// @112
BOOL STORMAPI SNetGetPlayerCaps(std::uint32_t playerid, SNETCAPSPTR caps) {
  return FALSE;
}

// @113
BOOL STORMAPI SNetGetPlayerName(std::uint32_t playerid, char* buffer, std::uint32_t buffersize) {
  return FALSE;
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
BOOL STORMAPI SNetGetTurnsInTransit(std::uint32_t* turns) {
  return FALSE;
}

// @116
BOOL STORMAPI SNetInitializeDevice(std::uint32_t deviceid, SNETPROGRAMDATAPTR programdata, SNETPLAYERDATAPTR playerdata, SNETUIDATAPTR interfacedata, SNETVERSIONDATAPTR versiondata) {
  return FALSE;
}

// @117
BOOL STORMAPI SNetInitializeProvider(std::uint32_t providerid, SNETPROGRAMDATAPTR programdata, SNETPLAYERDATAPTR playerdata, SNETUIDATAPTR interfacedata, SNETVERSIONDATAPTR versiondata) {
  return FALSE;
}

// @118
BOOL STORMAPI SNetJoinGame(std::uint32_t gameid, const char* gamename, const char* gamepassword, const char* playername, const char* playerdescription, std::uint32_t* playerid) {
  return FALSE;
}

// @119
BOOL STORMAPI SNetLeaveGame(std::uint32_t exitcode) {
  return FALSE;
}

// @120
BOOL STORMAPI SNetPerformUpgrade(std::uint32_t* upgradestatus) {
  return FALSE;
}

// @121
BOOL STORMAPI SNetReceiveMessage(std::uint32_t* senderplayerid, void** data, std::uint32_t* databytes) {
  return FALSE;
}

// @122
BOOL STORMAPI SNetReceiveTurns(std::uint32_t firstplayerid, std::uint32_t arraysize, void** arraydata, std::uint32_t* arraydatabytes, std::uint32_t* arrayplayerstatus) {
  return FALSE;
}

// @123
BOOL STORMAPI SNetRegisterEventHandler(std::uint32_t eventid, SNETEVENTPROC callback) {
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

// @125
BOOL STORMAPI SNetSelectGame(std::uint32_t flags, SNETPROGRAMDATAPTR programdata, SNETPLAYERDATAPTR playerdata, SNETUIDATAPTR interfacedata, SNETVERSIONDATAPTR versiondata, std::uint32_t* playerid) {
  return FALSE;
}

// @127
BOOL STORMAPI SNetSendMessage(std::uint32_t targetplayerid, void* data, std::uint32_t databytes) {
  return FALSE;
}

// @128
BOOL STORMAPI SNetSendTurn(void* data, std::uint32_t databytes) {
  return FALSE;
}

// @130
BOOL STORMAPI SNetSetGameMode(std::uint32_t modeflags) {
  return FALSE;
}

// @133
BOOL STORMAPI SNetEnumGamesEx(std::uint32_t categorybits, std::uint32_t categorymask, SNETENUMGAMESEXPROC callback, std::uint32_t* hintnextcall) {
  return FALSE;
}

// @134
BOOL STORMAPI SNetSendServerChatCommand(const char* command) {
  return FALSE;
}

// @137
BOOL STORMAPI SNetDisconnectAll(std::uint32_t flags) {
  SCOPE_LOCK(s_api_critsect);

  if (!s_spi) {
    SErrSetLastError(ERROR_BAD_PROVIDER);
    return FALSE;
  }

  if (s_game_playerid == -1) {
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
BOOL STORMAPI SNetCreateLadderGame(const char* gamename, const char* gamepassword, const char* gamedescription, std::uint32_t gamecategorybits, std::uint32_t ladderid, std::uint32_t gamemode, void* initdata, std::uint32_t initdatabytes, std::uint32_t maxplayers, const char* playername, const char* playerdescription, std::uint32_t* playerid) {
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
int STORMAPI SNetSendReplayPath(const char* replaypath, std::uint32_t gameid, const char* textgameresult) {
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
int STORMAPI SNetGetLeagueId(std::uint32_t* leagueID) {
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
std::uint32_t STORMAPI SNetGetCurrentProviderID() {
  return s_spi_providerptr ? s_spi_providerptr->id : 0;
}

// @148
void SNetSetCodeSignVerifcationFunction(CODEVERIFYPROC verifyfn) {
  s_verify_fn = verifyfn;
}
