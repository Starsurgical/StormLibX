#include <storm/Error.hpp>
#include <storm/Event.hpp>

#include <SMem.h>
#include <SNet.h>
#include "Net/Conn.hpp"
#include "Net/Game.hpp"
#include "Net/Perf.hpp"
#include "Net/Recv.hpp"
#include "Net/Spi.hpp"
#include "Net/Sys.hpp"

#include <mutex>
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

std::recursive_mutex s_api_critsect;
static CODEVERIFYPROC s_CodeSignFunc;
uint32_t s_api_playeroffset;

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

  RecvDestroy();

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
  STORM_VALIDATE_BEGIN;
  STORM_VALIDATE(callback);
  STORM_VALIDATE_END;

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
  STORM_VALIDATE_BEGIN;
  STORM_VALIDATE(caps && caps->size == sizeof(SNETCAPS));
  STORM_VALIDATE_END;

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
  STORM_VALIDATE_BEGIN;
  STORM_VALIDATE(caps && caps->size == sizeof(SNETCAPS));
  STORM_VALIDATE_END;

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
  STORM_VALIDATE_BEGIN;
  STORM_VALIDATE(turns);
  STORM_VALIDATE_END;

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
  STORM_VALIDATE_BEGIN;
  STORM_VALIDATE(callback);
  STORM_VALIDATE_END;

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
  STORM_VALIDATE_BEGIN;
  STORM_VALIDATE(data);
  STORM_VALIDATE(databytes);
  STORM_VALIDATE_END;

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
  STORM_VALIDATE_BEGIN;
  STORM_VALIDATE(data);
  STORM_VALIDATE(databytes);
  STORM_VALIDATE_END;

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
  STORM_VALIDATE_BEGIN;
  STORM_VALIDATE(callback);
  STORM_VALIDATE_END;

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
  STORM_VALIDATE_BEGIN;
  STORM_VALIDATE(command);
  STORM_VALIDATE(*command);
  STORM_VALIDATE_END;

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
  STORM_VALIDATE_BEGIN;
  STORM_VALIDATE(arraysize);
  STORM_VALIDATE(resultarray);
  STORM_VALIDATE_END;

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

  STORM_VALIDATE_BEGIN;
  STORM_VALIDATE(filename);
  STORM_VALIDATE(*filename);
  STORM_VALIDATE(data);
  STORM_VALIDATE(extendedresult);
  STORM_VALIDATE_END;

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
  STORM_VALIDATE_BEGIN;
  STORM_VALIDATE(cmd);
  STORM_VALIDATE_END;

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
  STORM_VALIDATE_BEGIN;
  STORM_VALIDATE(replaypath);
  STORM_VALIDATE_END;

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
  STORM_VALIDATE_BEGIN;
  STORM_VALIDATE(leagueID);
  STORM_VALIDATE_END;

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
  STORM_VALIDATE_BEGIN;
  STORM_VALIDATE(bnetName);
  STORM_VALIDATE_END;

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
  STORM_VALIDATE_BEGIN;
  STORM_VALIDATE(pszReplyName);
  STORM_VALIDATE(nameSize != 0);
  STORM_VALIDATE_END;

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
