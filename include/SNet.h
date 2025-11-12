#ifndef __STORMLIBX_SNET_H__
#define __STORMLIBX_SNET_H__

#include "StormTypes.h"

//###########################################################################
//## Defs
//###########################################################################

#define SNET_GM_PRIVATE     0x01
#define SNET_GM_FULL        0x02
#define SNET_GM_ADVERTISED  0x04
#define SNET_GM_UNJOINABLE  0x08
#define SNET_GM_REPLAY      0x80

#define SNET_PSF_ACTIVE        0x10000
#define SNET_PSF_TURNAVAILABLE 0x20000
#define SNET_PSF_RESPONDING    0x40000

#define SNET_BROADCASTNONLOCALPLAYERID  0xFFFFFFFE
#define SNET_BROADCASTPLAYERID          0xFFFFFFFF
#define SNET_INVALIDPLAYERID            0xFFFFFFFF

#define SNET_EVENT_INITDATA       1
#define SNET_EVENT_PLAYERJOIN     2
#define SNET_EVENT_PLAYERLEAVE    3
#define SNET_EVENT_SERVERMESSAGE  4

#define SNET_EXIT_AUTO_JOINING  0x00000001
#define SNET_EXIT_AUTO_NEWGAME  0x00000002
#define SNET_EXIT_AUTO_SHUTDOWN 0x00000003
#define SNET_EXIT_PLAYERQUIT    0x40000001
#define SNET_EXIT_PLAYERKILLED  0x40000002
#define SNET_EXIT_PLAYERWON     0x40000004
#define SNET_EXIT_GAMEOVER      0x40000005
#define SNET_EXIT_NOTRESPONDING 0x40000006

#define SNET_CAPS_PAGELOCKEDBUFFERS 0x00000001
#define SNET_CAPS_BASICINTERFACE    0x00000002
#define SNET_CAPS_DEBUGONLY         0x10000000
#define SNET_CAPS_RETAILONLY        0x20000000

#define SNET_UPGRADE_FAILED       -1
#define SNET_UPGRADE_NOT_NEEDED   0
#define SNET_UPGRADE_SUCCEEDED    1
#define SNET_UPGRADING_TERMINATE  2

#define SNET_INFO_GAMENAME        1
#define SNET_INFO_GAMEPASSWORD    2
#define SNET_INFO_GAMEDESCRIPTION 3
#define SNET_INFO_GAMEMODE        4
#define SNET_INFO_INITDATA        5
#define SNET_INFO_MAXPLAYERS      6

#define SNET_TRAFFIC_NORMAL 0
#define SNET_TRAFFIC_VERIFY 1
#define SNET_TRAFFIC_RESEND 2
#define SNET_TRAFFIC_REPLY  4

// values for arrayplayerstatus
#define SNET_PS_OK             0
#define SNET_PS_WAITING        2
#define SNET_PS_NOTRESPONDING  3
#define SNET_PS_UNKNOWN        default


//###########################################################################
//## Types
//###########################################################################

typedef struct _SNETCAPS {
  DWORD size;
  DWORD flags;
  DWORD maxmessagesize;
  DWORD maxqueuesize;
  DWORD maxplayers;
  DWORD bytessec;
  DWORD latencyms;
  DWORD defaultturnssec;
  DWORD defaultturnsintransit;
} SNETCAPS, * SNETCAPSPTR;

typedef struct _SNETCREATEDATA {
  DWORD size;
  DWORD providerid;
  DWORD maxplayers;
  DWORD createflags;
} SNETCREATEDATA, * SNETCREATEDATAPTR;

typedef struct _SNET_DATA_SYSCOLORTABLE {
  DWORD    syscolor;
  COLORREF rgb;
} SNET_DATA_SYSCOLORTABLE, * SNET_DATA_SYSCOLORTABLEPTR;

typedef struct _SNETEVENT {
  DWORD  eventid;
  DWORD  playerid;
  LPVOID data;
  DWORD  databytes;
} SNETEVENT, * SNETEVENTPTR;

typedef struct _SNETGAME {
  DWORD  size;
  DWORD  id;
  LPCSTR gamename;
  LPCSTR gamedescription;
  DWORD  categorybits;
  DWORD  numplayers;
  DWORD  maxplayers;
} SNETGAME, * SNETGAMEPTR;

typedef struct _SNETPLAYERDATA {
  DWORD size;
  LPCSTR playername;
  LPCSTR playerdescription;
  LPCSTR displayedfields;
} SNETPLAYERDATA, * SNETPLAYERDATAPTR;

typedef struct _SNETPROGRAMDATA {
  DWORD  size;
  LPCSTR programname;
  LPCSTR programdescription;
  DWORD  programid;
  DWORD  versionid;
  DWORD  reserved1;
  DWORD  maxplayers;
  LPVOID initdata;
  DWORD  initdatabytes;
  LPVOID reserved2;
  DWORD  optcategorybits;
  LPSTR  cdkey;
  LPSTR  registereduser;
  BOOL   spawned;
  DWORD  lcid;
} SNETPROGRAMDATA, * SNETPROGRAMDATAPTR;

typedef struct _SNETVERSIONDATA {
  DWORD size;
  LPCSTR versionstring;
  LPCSTR executablefile;
  LPCSTR originalarchivefile;
  LPCSTR patcharchivefile;
} SNETVERSIONDATA, * SNETVERSIONDATAPTR;


typedef struct _SNETUIDATA *SNETUIDATAPTR;

typedef BOOL(STORMAPI* SNETABORTPROC)();
typedef void (STORMAPI* SNETADDCATEGORYPROC)(LPCSTR, DWORD, DWORD);
typedef void (STORMAPI* SNETCATEGORYLISTPROC)(SNETPLAYERDATAPTR, SNETADDCATEGORYPROC);
typedef BOOL(STORMAPI* SNETCATEGORYPROC)(BOOL, SNETPROGRAMDATAPTR, SNETPLAYERDATAPTR, SNETUIDATAPTR, SNETVERSIONDATAPTR, DWORD*, DWORD*);
typedef BOOL(STORMAPI* SNETCHECKAUTHPROC)(DWORD, LPCSTR, LPCSTR, DWORD, LPCSTR, LPSTR, DWORD);
typedef BOOL(STORMAPI* SNETCREATEPROC)(SNETCREATEDATAPTR, SNETPROGRAMDATAPTR, SNETPLAYERDATAPTR, SNETUIDATAPTR, SNETVERSIONDATAPTR, DWORD*);
typedef BOOL(STORMAPI* SNETDRAWDESCPROC)(DWORD, DWORD, LPCSTR, LPCSTR, DWORD, DWORD, DWORD, LPDRAWITEMSTRUCT);
typedef BOOL(STORMAPI* SNETENUMDEVICESPROC)(DWORD, LPCSTR, LPCSTR);
typedef BOOL(STORMAPI* SNETENUMGAMESEXPROC)(SNETGAMEPTR);
typedef BOOL(STORMAPI* SNETENUMPROVIDERSPROC)(DWORD, LPCSTR, LPCSTR, SNETCAPSPTR);
typedef void (STORMAPI* SNETEVENTPROC)(SNETEVENTPTR);
typedef BOOL(STORMAPI* SNETGETARTPROC)(DWORD, DWORD, LPPALETTEENTRY, LPBYTE, DWORD, int*, int*, int*);
typedef BOOL(STORMAPI* SNETGETDATAPROC)(DWORD, DWORD, LPVOID, DWORD, DWORD*);
typedef int (STORMAPI* SNETMESSAGEBOXPROC)(HWND, LPCSTR, LPCSTR, UINT);
typedef BOOL(STORMAPI* SNETPLAYSOUNDPROC)(DWORD, DWORD, DWORD);
typedef BOOL(STORMAPI* SNETSELECTEDPROC)(DWORD, SNETCAPSPTR, _SNETUIDATA*, _SNETVERSIONDATA*);
typedef BOOL(STORMAPI* SNETSTATUSPROC)(LPCSTR, DWORD, DWORD, DWORD, SNETABORTPROC);
typedef BOOL(STORMAPI* SNETPROFILEPROC)();
typedef BOOL(STORMAPI* SNETNEWACCOUNTPROC)();

typedef bool(__fastcall* CODEVERIFYPROC)(LPCSTR);
typedef void(STORMAPI* SNETLEAGUECMDRESULTPROC)(LPCSTR, int);


// TODO: identify profilecallback, profilefields, profilebitmapcallback
typedef struct _SNETUIDATA {
  DWORD size;
  DWORD uiflags;
  HWND parentwindow;
  SNETGETARTPROC artcallback;
  SNETCHECKAUTHPROC authcallback;
  SNETCREATEPROC createcallback;
  SNETDRAWDESCPROC drawdesccallback;
  SNETSELECTEDPROC selectedcallback;
  SNETMESSAGEBOXPROC messageboxcallback;
  SNETPLAYSOUNDPROC soundcallback;
  SNETSTATUSPROC statuscallback;
  SNETGETDATAPROC getdatacallback;
  SNETCATEGORYPROC categorycallback;
  SNETCATEGORYLISTPROC categorylistcallback;
  SNETNEWACCOUNTPROC newaccountcallback;
  SNETPROFILEPROC profilecallback;
  void* profilerendercallback; // TODO
  void* profilesexcallback; // TODO
  void* templatecallback; // TODO
  void* sigvalidatecallback; // TODO
  void* leaguecallback; // TODO
} SNETUIDATA, * SNETUIDATAPTR;


//###########################################################################
//## SPI
//###########################################################################

#define  SNETSPI_MAXCLIENTDATA    256
#define  SNETSPI_MAXSTRINGLENGTH  128

typedef struct _SNETADDR {
  BYTE address[16];
} SNETADDR, * SNETADDRPTR;

typedef struct _SNETSPI_DEVICELIST {
  DWORD                deviceid;
  SNETCAPS             devicecaps;
  char                 devicename[SNETSPI_MAXSTRINGLENGTH];
  char                 devicedescription[SNETSPI_MAXSTRINGLENGTH];
  DWORD                reserved;
  _SNETSPI_DEVICELIST* next;
} SNETSPI_DEVICELIST, * SNETSPI_DEVICELISTPTR;

typedef struct _SNETSPI_GAMELIST {
  DWORD              gameid;
  DWORD              gamemode;
  DWORD              creationtime;
  SNETADDR           owner;
  DWORD              ownerlatency;
  DWORD              ownerlasttime;
  DWORD              gamecategorybits;
  char               gamename[SNETSPI_MAXSTRINGLENGTH];
  char               gamedescription[SNETSPI_MAXSTRINGLENGTH];
  _SNETSPI_GAMELIST* next;
  LPVOID             clientdata;
  DWORD              clientdatabytes;
  DWORD              productid;
  DWORD              version;
} SNETSPI_GAMELIST, * SNETSPI_GAMELISTPTR;

typedef struct _SNETSPI {
  // The size of the vtable
  DWORD size;
  // Compares two sockaddrs with each other and returns the number of differences in dwResult
  BOOL(STORMAPI* spiCompareNetAddresses)(SNETADDRPTR addr1, SNETADDRPTR addr2, DWORD* diffmagnitude);
  // Called when the module is released
  BOOL(STORMAPI* spiDestroy)();
  // Called in order to free blocks of packet memory returned in the spiReceive functions
  BOOL(STORMAPI* spiFree)(SNETADDRPTR addr, LPVOID data, DWORD databytes);
  BOOL(STORMAPI* spiFreeExternalMessage)(LPCSTR addr, LPCSTR data, LPCSTR databytes); // TODO: possibly incorrect, investigate
  // Returns info on a specified game
  BOOL(STORMAPI* spiGetGameInfo)(DWORD gameid, LPCSTR gamename, LPCSTR gamepassword, SNETSPI_GAMELIST* gameinfo);
  // Returns packet statistics
  BOOL(STORMAPI* spiGetPerformanceData)(DWORD counterid, DWORD* countervalue, LARGE_INTEGER* measurementtime, LARGE_INTEGER* measurementfreq);
  // Called when the module is initialized
  BOOL(STORMAPI* spiInitialize)(SNETPROGRAMDATAPTR programdata, SNETPLAYERDATAPTR playerdata, SNETUIDATAPTR interfacedata, SNETVERSIONDATAPTR versiondata, HANDLE hEvent);
  BOOL(STORMAPI* spiInitializeDevice)(DWORD deviceid, SNETPROGRAMDATAPTR programdata, SNETPLAYERDATAPTR playerdata, SNETUIDATAPTR itnerfacedata, SNETVERSIONDATAPTR versiondata);
  BOOL(STORMAPI* spiLockDeviceList)(SNETSPI_DEVICELISTPTR* devicelist);
  // Called to prevent the game list from updating so that it can be processed by storm
  BOOL(STORMAPI* spiLockGameList)(DWORD categorybits, DWORD categorymask, SNETSPI_GAMELISTPTR* gamelist);
  // Return received data from a connectionless socket to storm
  BOOL(STORMAPI* spiReceive)(SNETADDRPTR* addr, LPVOID* data, DWORD* databytes);
  // Return received data from a connected socket to storm
  BOOL(STORMAPI* spiReceiveExternalMessage)(SNETADDRPTR* addr, LPVOID* data, DWORD* databytes); // TODO: possibly incorrect, investigate
  // Called when a game is selected to query information
  BOOL(STORMAPI* spiSelectGame)(DWORD flags, SNETPROGRAMDATAPTR programdata, SNETPLAYERDATAPTR playerdata, SNETUIDATAPTR interfacedata, SNETVERSIONDATAPTR versiondata, DWORD* playerid);
  // Sends data over a connectionless socket
  BOOL(STORMAPI* spiSend)(DWORD addresses, SNETADDRPTR* addrlist, LPVOID data, DWORD databytes);
  // Sends data over a connected socket
  BOOL(STORMAPI* spiSendExternalMessage)(LPCSTR senderpath, LPCSTR sendername, LPCSTR targetpath, LPCSTR targetname, LPCSTR message);
  // An extended version of spiStartAdvertisingGame
  // BOOL(STORMAPI* spiStartAdvertisingGame)(LPCSTR gamename, LPCSTR gamepassword, LPCSTR gamedescription, DWORD gamemode, DWORD gameage, DWORD gamecategorybits, DWORD optcategorybits, LPCVOID clientdata, DWORD clientdatabytes); <-- old
  BOOL(STORMAPI* spiStartAdvertisingGame)(LPCSTR gamename, LPCSTR gamepassword, LPCSTR gamedescription, DWORD gamemode, DWORD gameage, DWORD gamecategorybits, DWORD optcategorybits, DWORD, LPCVOID clientdata, DWORD clientdatabytes);
  // Called to stop advertising the game
  BOOL(STORMAPI* spiStopAdvertisingGame)();
  BOOL(STORMAPI* spiUnlockDeviceList)(SNETSPI_DEVICELISTPTR devicelist);
  // Called after the game list has been processed and resume updating
  BOOL(STORMAPI* spiUnlockGameList)(SNETSPI_GAMELISTPTR gamelist, DWORD* hintnextcall);
  BOOL(STORMAPI* spiGetLocalPlayerName)(LPCSTR namebuffer, DWORD namechars, LPCSTR descbuffer, DWORD descchars);
  BOOL(STORMAPI* spiReportGameResult)(DWORD ladderid, DWORD arraysize, LPCSTR *namearray, DWORD *resultarray, LPCSTR textgameresult, LPCSTR textplayerresult);
  BOOL(STORMAPI* spiCheckDataFile)(LPCSTR filename, const void *data, DWORD bytes, DWORD *extendedresult);
  BOOL(STORMAPI* spiSendLeagueCommand)(LPCSTR cmd, void* callback);
  BOOL(STORMAPI* spiSendReplayPath)(LPCSTR replayPath, int, int);
  BOOL(STORMAPI* spiGetLeagueId)(DWORD *pid);
  BOOL(STORMAPI* spiLeagueLogout)(LPCSTR bnetName);
  // Retrieves the player name that last whispered you on battle.net
  BOOL(STORMAPI* spiGetReplyName)(LPCSTR buffer, DWORD buffersize);
} SNETSPI, *SNETSPIPTR;

typedef BOOL(STORMAPI* SNETSPIBIND)(DWORD, SNETSPIPTR*);
typedef BOOL(STORMAPI* SNETSPIQUERY)(DWORD, DWORD*, LPCSTR*, LPCSTR*, SNETCAPSPTR*);


//###########################################################################
//## Functions
//###########################################################################

extern "C" {

  // @101
  BOOL STORMAPI SNetCreateGame(LPCSTR gamename, LPCSTR gamepassword, LPCSTR gamedescription, DWORD gamecategorybits, LPVOID initdata, DWORD initdatabytes, DWORD maxplayers, LPCSTR playername, LPCSTR playerdescription, DWORD* playerid);

  // @102
  BOOL STORMAPI SNetDestroy();

  // @103
  BOOL STORMAPI SNetEnumDevices(SNETENUMDEVICESPROC callback);

  // @105
  BOOL STORMAPI SNetEnumProviders(SNETCAPSPTR mincaps, SNETENUMPROVIDERSPROC callback);

  // @106
  BOOL STORMAPI SNetDropPlayer(DWORD playerid, DWORD exitcode);

  // @107
  BOOL STORMAPI SNetGetGameInfo(DWORD index, LPVOID buffer, DWORD buffersize, DWORD* byteswritten);

  // @109
  BOOL STORMAPI SNetGetNumPlayers(DWORD* firstplayerid, DWORD* lastplayerid, DWORD* activeplayers);

  // @112
  BOOL STORMAPI SNetGetPlayerCaps(DWORD playerid, SNETCAPSPTR caps);

  // @113
  BOOL STORMAPI SNetGetPlayerName(DWORD playerid, LPSTR buffer, DWORD buffersize);

  // @114
  BOOL STORMAPI SNetGetProviderCaps(SNETCAPSPTR caps);

  // @115
  BOOL STORMAPI SNetGetTurnsInTransit(DWORD* turns);

  // @116
  BOOL STORMAPI SNetInitializeDevice(DWORD deviceid, SNETPROGRAMDATAPTR programdata, SNETPLAYERDATAPTR playerdata, SNETUIDATAPTR interfacedata, SNETVERSIONDATAPTR versiondata);

  // @117
  BOOL STORMAPI SNetInitializeProvider(DWORD providerid, SNETPROGRAMDATAPTR programdata, SNETPLAYERDATAPTR playerdata, SNETUIDATAPTR interfacedata, SNETVERSIONDATAPTR versiondata);

  // @118
  BOOL STORMAPI SNetJoinGame(DWORD gameid, LPCSTR gamename, LPCSTR gamepassword, LPCSTR playername, LPCSTR playerdescription, DWORD* playerid);

  // @119
  BOOL STORMAPI SNetLeaveGame(DWORD exitcode);

  // @120
  BOOL STORMAPI SNetPerformUpgrade(DWORD* upgradestatus);

  // @121
  BOOL STORMAPI SNetReceiveMessage(DWORD* senderplayerid, LPVOID* data, DWORD* databytes);

  // @122
  BOOL STORMAPI SNetReceiveTurns(DWORD firstplayerid, DWORD arraysize, LPVOID* arraydata, LPDWORD arraydatabytes, LPDWORD arrayplayerstatus);

  // @123
  BOOL STORMAPI SNetRegisterEventHandler(DWORD eventid, SNETEVENTPROC callback);

  // @125
  BOOL STORMAPI SNetSelectGame(DWORD flags, SNETPROGRAMDATAPTR programdata, SNETPLAYERDATAPTR playerdata, SNETUIDATAPTR interfacedata, SNETVERSIONDATAPTR versiondata, DWORD* playerid);

  // @127
  BOOL STORMAPI SNetSendMessage(DWORD targetplayerid, LPVOID data, DWORD databytes);

  // @128
  BOOL STORMAPI SNetSendTurn(LPVOID data, DWORD databytes);

  // @130
  BOOL STORMAPI SNetSetGameMode(DWORD modeflags);

  // @133
  BOOL STORMAPI SNetEnumGamesEx(DWORD categorybits, DWORD categorymask, SNETENUMGAMESEXPROC callback, DWORD* hintnextcall);

  // @134
  BOOL STORMAPI SNetSendServerChatCommand(LPCSTR command);

  // @137
  BOOL STORMAPI SNetDisconnectAll(DWORD flags);

  // @138
  BOOL STORMAPI SNetCreateLadderGame(LPCSTR gamename, LPCSTR gamepassword, LPCSTR gamedescription, DWORD gamecategorybits, DWORD ladderid, DWORD gamemode, LPVOID initdata, DWORD initdatabytes, DWORD maxplayers, LPCSTR playername, LPCSTR playerdescription, DWORD* playerid);

  // @139
  BOOL STORMAPI SNetReportGameResult(unsigned firstplayerid, int arraysize, int* resultarray, const char* textgameresult, const char* textplayerresult);

  // @141
  BOOL STORMAPI SNetSendLeagueCommand(LPCTSTR cmd, SNETLEAGUECMDRESULTPROC callback);

  // @142
  int STORMAPI SNetSendReplayPath(int a1, int a2, char* replayPath);

  // @143
  int STORMAPI SNetGetLeagueName(int leagueID);

  // @144
  BOOL STORMAPI SNetGetPlayerNames(LPSTR* names);

  // @145
  int STORMAPI SNetLeagueLogout(char* bnetName);

  // @146
  int STORMAPI SNetGetReplyName(char* pszReplyName, size_t nameSize);

  // @147
  // Returns 4 byte protocol identifier of current protocol, only used for debugging, can be removed.
  DWORD STORMAPI SNetGetProtocol();

  // @148
  void SNetSetCodeSignVerifcationFunction(CODEVERIFYPROC);
}

#endif
