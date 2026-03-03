#include "Game.hpp"
#include "Spi.hpp"
#include "Win.hpp"

#include <Storm/Error.hpp>
#include <Storm/String.hpp>
#include <SNet.h>

#include <memory>
#include <vector>

std::vector<_PLAYERNAME> s_game_playernames;
uint32_t s_game_categorybits;
uint32_t s_game_creationtime;
void* s_game_initdata;
uint32_t s_game_initdatabytes;
uint32_t s_game_gamemode;
char s_game_gamedesc[SNETSPI_MAXSTRINGLENGTH];
char s_game_gamename[SNETSPI_MAXSTRINGLENGTH];
char s_game_gamepass[SNETSPI_MAXSTRINGLENGTH];
uint32_t s_game_joining;
uint32_t s_game_ladderid;
uint32_t s_game_optcategorybits;
uint32_t s_game_playersallowed;
uint32_t s_game_programid;
uint32_t s_game_versionid;
uint8_t s_game_playerid = NOPLAYER;


void GameBuildClientData(CLIENTDATAPTR buffer, uint32_t* bytes) {
  buffer->bytes = sizeof(CLIENTDATA);
  SNetGetNumPlayers(nullptr, nullptr, &buffer->numplayers);
  buffer->maxplayers = s_game_playersallowed;
  *bytes = buffer->bytes;
}

void GameSetPlayerName(uint32_t id, const char *name) {
  STORM_VALIDATE_BEGIN;
  STORM_VALIDATE(id < 256);
  STORM_VALIDATE_END;

  if (id + 1 > s_game_playernames.size()) {
    s_game_playernames.resize(id + 1, {});
  }

  SStrCopy(s_game_playernames[id].name, name, sizeof(_PLAYERNAME::name));
}

int32_t GameStartAdvertising() {
  CLIENTDATA clientdata;
  uint32_t clientdatabytes;
  GameBuildClientData(&clientdata, &clientdatabytes);
  uint32_t gameage = (PortGetTickCount() - s_game_creationtime) / 1000;
  return s_spi->spiStartAdvertisingGame(s_game_gamename, s_game_gamepass, s_game_gamedesc, s_game_gamemode, gameage, s_game_categorybits, s_game_optcategorybits, s_game_ladderid, &clientdata, clientdatabytes);
}
