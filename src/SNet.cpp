#include <storm/Memory.hpp>
#include "SErr.h"
#include "SNet.h"
#include "SDraw.h"

#include <SDL.h>

#include <filesystem>
#include <mutex>
#include <vector>

#define CONCAT(a, b) CONCAT_INNER(a, b)
#define CONCAT_INNER(a, b) a ## b
#define SCOPE_LOCK(x) std::lock_guard<decltype(x)> CONCAT(x, __COUNTER__)(x)

/*
caps.dat

  struct {
    DWORD entrysize;
    DWORD providerid;
    char description[]; // null terminated string of any length
    SNETCAPS caps;
  }[]; // repeats for the number of providers
*/

namespace {
  struct PROVIDERINFO {
    char     filename[MAX_PATH];
    DWORD    index;
    DWORD    id;
    char     desc[SNETSPI_MAXSTRINGLENGTH];
    char     req[SNETSPI_MAXSTRINGLENGTH];
    SNETCAPS caps;
  };

  std::recursive_mutex s_spi_critsect;

  SNETSPIPTR s_spi;
  std::vector<PROVIDERINFO> s_spi_providerlist;
  PROVIDERINFO* s_spi_providerptr;

  CODEVERIFYPROC s_verify_fn;


  int STORMAPI SMessageBox(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType) {
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
}

// @101
BOOL STORMAPI SNetCreateGame(LPCSTR gamename, LPCSTR gamepassword, LPCSTR gamedescription, DWORD gamecategorybits, LPVOID initdata, DWORD initdatabytes, DWORD maxplayers, LPCSTR playername, LPCSTR playerdescription, DWORD* playerid) {
  return SNetCreateLadderGame(gamename, gamepassword, gamedescription, gamecategorybits, 0, 0, initdata, initdatabytes, maxplayers, playername, playerdescription, playerid);
}

// @102
BOOL STORMAPI SNetDestroy() {
  return TRUE;
}

// @103
BOOL STORMAPI SNetEnumDevices(SNETENUMDEVICESPROC callback) {
  VALIDATEBEGIN;
  VALIDATE(callback);
  VALIDATEEND;

  std::vector<SNETSPI_DEVICELIST> local_device_list;

  {
    SCOPE_LOCK(s_spi_critsect);

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
BOOL STORMAPI SNetDropPlayer(DWORD playerid, DWORD exitcode) {
  return FALSE;
}

// @107
BOOL STORMAPI SNetGetGameInfo(DWORD index, LPVOID buffer, DWORD buffersize, DWORD* byteswritten) {
  return FALSE;
}

// @109
BOOL STORMAPI SNetGetNumPlayers(DWORD* firstplayerid, DWORD* lastplayerid, DWORD* activeplayers) {
  return FALSE;
}

// @112
BOOL STORMAPI SNetGetPlayerCaps(DWORD playerid, SNETCAPSPTR caps) {
  return FALSE;
}

// @113
BOOL STORMAPI SNetGetPlayerName(DWORD playerid, LPSTR buffer, DWORD buffersize) {
  return FALSE;
}

// @114
BOOL STORMAPI SNetGetProviderCaps(SNETCAPSPTR caps) {
  VALIDATEBEGIN;
  VALIDATE(caps && caps->size == sizeof(SNETCAPS));
  VALIDATEEND;

  SCOPE_LOCK(s_spi_critsect);

  *caps = { sizeof(SNETCAPS) };

  if (!s_spi || !s_spi_providerptr) {
    SErrSetLastError(ERROR_BAD_PROVIDER);
    return FALSE;
  }

  *caps = s_spi_providerptr->caps;
  return TRUE;
}

// @115
BOOL STORMAPI SNetGetTurnsInTransit(DWORD* turns) {
  return FALSE;
}

// @116
BOOL STORMAPI SNetInitializeDevice(DWORD deviceid, SNETPROGRAMDATAPTR programdata, SNETPLAYERDATAPTR playerdata, SNETUIDATAPTR interfacedata, SNETVERSIONDATAPTR versiondata) {
  return FALSE;
}

// @117
BOOL STORMAPI SNetInitializeProvider(DWORD providerid, SNETPROGRAMDATAPTR programdata, SNETPLAYERDATAPTR playerdata, SNETUIDATAPTR interfacedata, SNETVERSIONDATAPTR versiondata) {
  return FALSE;
}

// @118
BOOL STORMAPI SNetJoinGame(DWORD gameid, LPCSTR gamename, LPCSTR gamepassword, LPCSTR playername, LPCSTR playerdescription, DWORD* playerid) {
  return FALSE;
}

// @119
BOOL STORMAPI SNetLeaveGame(DWORD exitcode) {
  return FALSE;
}

// @120
BOOL STORMAPI SNetPerformUpgrade(DWORD* upgradestatus) {
  return FALSE;
}

// @121
BOOL STORMAPI SNetReceiveMessage(DWORD* senderplayerid, LPVOID* data, DWORD* databytes) {
  return FALSE;
}

// @122
BOOL STORMAPI SNetReceiveTurns(DWORD firstplayerid, DWORD arraysize, LPVOID* arraydata, LPDWORD arraydatabytes, LPDWORD arrayplayerstatus) {
  return FALSE;
}

// @123
BOOL STORMAPI SNetRegisterEventHandler(DWORD eventid, SNETEVENTPROC callback) {
  return FALSE;
}

// @125
BOOL STORMAPI SNetSelectGame(DWORD flags, SNETPROGRAMDATAPTR programdata, SNETPLAYERDATAPTR playerdata, SNETUIDATAPTR interfacedata, SNETVERSIONDATAPTR versiondata, DWORD* playerid) {
  return FALSE;
}

// @127
BOOL STORMAPI SNetSendMessage(DWORD targetplayerid, LPVOID data, DWORD databytes) {
  return FALSE;
}

// @128
BOOL STORMAPI SNetSendTurn(LPVOID data, DWORD databytes) {
  return FALSE;
}

// @130
BOOL STORMAPI SNetSetGameMode(DWORD modeflags) {
  return FALSE;
}

// @133
BOOL STORMAPI SNetEnumGamesEx(DWORD categorybits, DWORD categorymask, SNETENUMGAMESEXPROC callback, DWORD* hintnextcall) {
  return FALSE;
}

// @134
BOOL STORMAPI SNetSendServerChatCommand(LPCSTR command) {
  return FALSE;
}

// @137
BOOL STORMAPI SNetDisconnectAll(DWORD flags) {
  return FALSE;
}

// @138
BOOL STORMAPI SNetCreateLadderGame(LPCSTR gamename, LPCSTR gamepassword, LPCSTR gamedescription, DWORD gamecategorybits, DWORD ladderid, DWORD gamemode, LPVOID initdata, DWORD initdatabytes, DWORD maxplayers, LPCSTR playername, LPCSTR playerdescription, DWORD* playerid) {
  return FALSE;
}

// @139
BOOL STORMAPI SNetReportGameResult(unsigned firstplayerid, int arraysize, int* resultarray, const char* textgameresult, const char* textplayerresult) {
  return TRUE;
}

// @141
BOOL STORMAPI SNetSendLeagueCommand(LPCSTR cmd, SNETLEAGUECMDRESULTPROC callback) {
  VALIDATEBEGIN;
  VALIDATE(cmd);
  VALIDATEEND;

  SCOPE_LOCK(s_spi_critsect);

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
int STORMAPI SNetSendReplayPath(int a1, int a2, char* replayPath) {
  return 0;
}

// @143
int STORMAPI SNetGetLeagueName(int leagueID) {
  return 0;
}

// @144
BOOL STORMAPI SNetGetPlayerNames(LPSTR* names) {
  return FALSE;
}

// @145
int STORMAPI SNetLeagueLogout(char* bnetName) {
  VALIDATEBEGIN;
  VALIDATE(bnetName);
  VALIDATEEND;

  SCOPE_LOCK(s_spi_critsect);

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
  return 0;
}

// @147
// Returns 4 byte protocol identifier of current protocol, only used for debugging, can be removed.
DWORD STORMAPI SNetGetProtocol() {
  return s_spi_providerptr ? s_spi_providerptr->id : 0;
}

// @148
void SNetSetCodeSignVerifcationFunction(CODEVERIFYPROC verifyfn) {
  s_verify_fn = verifyfn;
}
