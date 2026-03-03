#include "Sys.hpp"

#include <Storm/Event.hpp>
#include <Storm/List.hpp>
#include <Storm/String.hpp>
#include <SMem.h>
#include "Conn.hpp"
#include "Game.hpp"
#include "Perf.hpp"
#include "Recv.hpp"
#include "Win.hpp"

#include <mutex>


#define CONCAT(a, b) CONCAT_INNER(a, b)
#define CONCAT_INNER(a, b) a ## b
#define SCOPE_LOCK(x) std::lock_guard<decltype(x)> CONCAT(x, __COUNTER__)(x)


std::recursive_mutex s_sys_usereventlist_critsect;
STORM_LIST(USEREVENT) s_sys_usereventlist;
bool s_sys_event[SYSMSGS];

extern uint32_t s_api_playeroffset;


uint32_t SysBuildPlayerInfo(SYSEVENTDATA_PLAYERINFOPTR data, CONNREC* conn, uint32_t startingturn) {
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

void STORMAPI SysOnCircuitCheck(SYSEVENTPTR event) {
  if (event->databytes == sizeof(uint32_t) && *static_cast<uint32_t*>(event->data) == 1) {
    ConnSendMessage(ConnFindByAddr(event->senderaddr), TYPE_SYSTEM, SYS_CIRCUITCHECKRESPONSE, event->data, 4);
  }
}

void STORMAPI SysOnDropPlayer(SYSEVENTPTR event) {
  SYSEVENTDATA_DROPPLAYERPTR msg = static_cast<SYSEVENTDATA_DROPPLAYERPTR>(event->data);
  CONNREC* rec = ConnFindByPlayerId(msg->playerid);
  if (rec && rec->playerid != NOPLAYER) {
    rec->flags |= PF_LEAVING;
    rec->finalsequence = msg->finalsequence;
    rec->exitcode = msg->exitcode;
  }
}

void STORMAPI SysOnNewGameMode(SYSEVENTPTR event) {
  s_game_gamemode = *static_cast<uint32_t*>(event->data);
  s_game_gamepass[0] = (s_game_gamemode & 1) ? s_game_gamepass[0] : '\0';
}

void STORMAPI SysOnNewGameOwner(SYSEVENTPTR event) {
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

void STORMAPI SysOnPing(SYSEVENTPTR event) {
  ConnSendMessage(ConnFindByAddr(event->senderaddr), TYPE_SYSTEM, SYS_PINGRESPONSE, event->data, event->databytes);
}

void STORMAPI SysOnPingResponse(SYSEVENTPTR event) {
  CONNREC* rec = ConnFindByAddr(event->senderaddr);
  uint32_t ticks = PortGetTickCount();
  if (rec) {
    rec->latency = ticks - rec->lastpingtime;
    rec->peaklatency = std::max(rec->latency, rec->peaklatency);
  }
}

void SysQueueUserEvent(uint32_t eventid, uint32_t playerid, const void* data, uint32_t databytes) {
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

void STORMAPI SysOnPlayerJoinAcceptDone(SYSEVENTPTR event) {
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

void STORMAPI SysOnPlayerJoinAcceptStart(SYSEVENTPTR event) {
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

void STORMAPI SysOnPlayerLeave(SYSEVENTPTR event) {
  SYSEVENTDATA_PLAYERLEAVEPTR msg = static_cast<SYSEVENTDATA_PLAYERLEAVEPTR>(event->data);
  CONNREC* rec = ConnFindByAddr(event->senderaddr);
  if (rec && rec->playerid != NOPLAYER) {
    rec->flags |= PF_LEAVING;
    rec->finalsequence = msg->finalsequence;
    rec->exitcode = msg->exitcode;
  }
}

void STORMAPI SysOnNewLadderId(SYSEVENTPTR event) {
  s_game_ladderid = *static_cast<uint32_t*>(event->data);
}

void STORMAPI SysOnPlayerJoinReject(SYSEVENTPTR event) {
  // intentionally empty
}

void SysDestroy() {
  SCOPE_LOCK(s_sys_usereventlist_critsect);
  while (USEREVENT* curr = s_sys_usereventlist.Head()) {
    if (curr->event.data) FREE(curr->event.data);
    s_sys_usereventlist.DeleteNode(curr);
  }
}

void SysDispatchUserEvents() {
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

int32_t SysWaitForMultipleEvents(uint32_t numevents, uint32_t* eventlist, int32_t waitforall, uint32_t timeout) {
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
