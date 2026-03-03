#include "Conn.hpp"
#include "Perf.hpp"
#include "Spi.hpp"
#include "Win.hpp"
#include <SMem.h>

STORM_LIST(CONNREC) s_conn_local;
STORM_LIST(CONNREC) s_conn_connlist;


void ConnAssignPlayerId(CONNREC* conn, uint8_t playerid) {
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

CONNREC* ConnFindByPlayerId(unsigned int playerid) {
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

CONNREC* ConnFindByOldPlayerId(unsigned int playerid) {
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

CONNREC* ConnAddRec(STORM_LIST(CONNREC)* list, SNETADDRPTR addr) {
  CONNREC *newconn = list->NewNode(STORM_LIST_TAIL, 0, 0);
  newconn->addr = *addr;
  newconn->playerid = NOPLAYER;
  newconn->oldplayerid = NOPLAYER;
  newconn->lastreceivetime = PortGetTickCount();
  return newconn;
}

CONNREC* ConnFindByAddr(SNETADDRPTR addr) {
  for (CONNREC *curr = s_conn_connlist.Head(); curr; curr = curr->Next()) {
    if (memcmp(&curr->addr, addr, sizeof(SNETADDR)) == 0) {
      return curr;
    }
  }
  return ConnAddRec(&s_conn_connlist, addr);
}

CONNREC* ConnFindLocal() {
  if (!s_conn_local.IsEmpty()) {
    return s_conn_local.Head();
  }

  SNETADDR addr = {};
  return ConnAddRec(&s_conn_local, &addr);
}

void ConnDestroyQueue(STORM_LIST(MESSAGE)* queue) {
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

void ConnSetCurrentMessage(CONNREC* conn, uint8_t type, MESSAGE* message) {
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

void ConnClearOldTurns(CONNREC* onlyconn) {
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

void ConnFree(CONNREC *conn) {
  for (int i = 0; i < TYPES; i++) {
    ConnDestroyQueue(&conn->outgoingqueue[i]);
    ConnDestroyQueue(&conn->incomingqueue[i]);
    ConnDestroyQueue(&conn->processing[i]);
  }
  ConnDestroyQueue(&conn->oldturns);
  delete conn;
}

void ConnSendPacket(CONNREC* conn, PACKETPTR pkt) {
  SNETADDRPTR addr = &conn->addr;
  SpiSend(1, &addr, pkt, pkt->header.bytes);
  conn->acksequence[pkt->header.type] = pkt->header.acksequence;
  conn->acktime[pkt->header.type] = 0;
}

void ConnProcessAck(CONNREC* conn, uint8_t type, uint16_t acksequence) {
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

void ConnResendMessage(CONNREC* conn, PACKETPTR data, uint32_t databytes) {
  PACKETPTR localpkt = static_cast<PACKETPTR>(ALLOC(databytes));
  SMemCopy(localpkt, data, databytes);

  uint32_t bytes = localpkt->header.bytes;
  localpkt->header.acksequence = conn->availablesequence[localpkt->header.type];
  localpkt->header.checksum = PktGenerateChecksum(localpkt);

  ConnSendPacket(conn, localpkt);
  FREE(localpkt);
}

MESSAGE* ConnSendMessage(CONNREC* target, uint8_t type, uint8_t subtype, void* data, uint32_t databytes) {
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

uint32_t ConnMaintainConnections() {
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

void ConnDestroy() {
  while (CONNREC* curr = s_conn_local.Head()) {
    ConnFree(curr);
  }

  while (CONNREC* curr = s_conn_connlist.Head()) {
    ConnFree(curr);
  }
}
