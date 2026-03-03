#ifndef __STORMLIBX_NET_GAME_H__
#define __STORMLIBX_NET_GAME_H__

#include <cstdint>
#include <vector>
#include <SNet.h>

struct _PLAYERNAME {
  char name[SNETSPI_MAXSTRINGLENGTH];
};

extern void* s_game_initdata;
extern uint32_t s_game_gamemode;
extern char s_game_gamedesc[SNETSPI_MAXSTRINGLENGTH];
extern char s_game_gamename[SNETSPI_MAXSTRINGLENGTH];
extern char s_game_gamepass[SNETSPI_MAXSTRINGLENGTH];
extern uint8_t s_game_playerid;
extern uint32_t s_game_initdatabytes;
extern uint32_t s_game_playersallowed;
extern uint32_t s_game_creationtime;
extern uint32_t s_game_ladderid;
extern std::vector<_PLAYERNAME> s_game_playernames;

extern uint32_t s_game_optcategorybits;
extern uint32_t s_game_programid;
extern uint32_t s_game_versionid;

void GameSetPlayerName(uint32_t id, const char *name);
int32_t GameStartAdvertising();

#endif
