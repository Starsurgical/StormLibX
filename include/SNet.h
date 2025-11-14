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
  std::uint32_t size;
  std::uint32_t flags;
  std::uint32_t maxmessagesize;
  std::uint32_t maxqueuesize;
  std::uint32_t maxplayers;
  std::uint32_t bytessec;
  std::uint32_t latencyms;
  std::uint32_t defaultturnssec;
  std::uint32_t defaultturnsintransit;
} SNETCAPS, * SNETCAPSPTR;

typedef struct _SNETCREATEDATA {
  std::uint32_t size;
  std::uint32_t providerid;
  std::uint32_t maxplayers;
  std::uint32_t createflags;
} SNETCREATEDATA, * SNETCREATEDATAPTR;

typedef struct _SNET_DATA_SYSCOLORTABLE {
  std::uint32_t    syscolor;
  COLORREF rgb;
} SNET_DATA_SYSCOLORTABLE, * SNET_DATA_SYSCOLORTABLEPTR;

typedef struct _SNETEVENT {
  std::uint32_t  eventid;
  std::uint32_t  playerid;
  void* data;
  std::uint32_t  databytes;
} SNETEVENT, * SNETEVENTPTR;

typedef struct _SNETGAME {
  std::uint32_t  size;
  std::uint32_t  id;
  const char* gamename;
  const char* gamedescription;
  std::uint32_t  categorybits;
  std::uint32_t  numplayers;
  std::uint32_t  maxplayers;
} SNETGAME, * SNETGAMEPTR;

typedef struct _SNETPLAYERDATA {
  std::uint32_t size;
  const char* playername;
  const char* playerdescription;
  const char* displayedfields;
} SNETPLAYERDATA, * SNETPLAYERDATAPTR;

typedef struct _SNETPROGRAMDATA {
  std::uint32_t  size;
  const char* programname;
  const char* programdescription;
  std::uint32_t  programid;
  std::uint32_t  versionid;
  std::uint32_t  reserved1;
  std::uint32_t  maxplayers;
  void* initdata;
  std::uint32_t  initdatabytes;
  void* reserved2;
  std::uint32_t  optcategorybits;
  char*  cdkey;
  char*  registereduser;
  BOOL   spawned;
  std::uint32_t  lcid;
} SNETPROGRAMDATA, * SNETPROGRAMDATAPTR;

typedef struct _SNETVERSIONDATA {
  std::uint32_t size;
  const char* versionstring;
  const char* executablefile;
  const char* originalarchivefile;
  const char* patcharchivefile;
} SNETVERSIONDATA, * SNETVERSIONDATAPTR;


typedef struct _SNETUIDATA *SNETUIDATAPTR;

typedef BOOL(STORMAPI* SNETABORTPROC)();
typedef void (STORMAPI* SNETADDCATEGORYPROC)(const char*, std::uint32_t, std::uint32_t);
typedef void (STORMAPI* SNETCATEGORYLISTPROC)(SNETPLAYERDATAPTR, SNETADDCATEGORYPROC);
typedef BOOL(STORMAPI* SNETCATEGORYPROC)(BOOL, SNETPROGRAMDATAPTR, SNETPLAYERDATAPTR, SNETUIDATAPTR, SNETVERSIONDATAPTR, std::uint32_t*, std::uint32_t*);
typedef BOOL(STORMAPI* SNETCHECKAUTHPROC)(std::uint32_t, const char*, const char*, std::uint32_t, const char*, char*, std::uint32_t);
typedef BOOL(STORMAPI* SNETCREATEPROC)(SNETCREATEDATAPTR, SNETPROGRAMDATAPTR, SNETPLAYERDATAPTR, SNETUIDATAPTR, SNETVERSIONDATAPTR, std::uint32_t*);
typedef BOOL(STORMAPI* SNETDRAWDESCPROC)(std::uint32_t, std::uint32_t, const char*, const char*, std::uint32_t, std::uint32_t, std::uint32_t, LPDRAWITEMSTRUCT);
typedef BOOL(STORMAPI* SNETENUMDEVICESPROC)(std::uint32_t, const char*, const char*);
typedef BOOL(STORMAPI* SNETENUMGAMESEXPROC)(SNETGAMEPTR);
typedef BOOL(STORMAPI* SNETENUMPROVIDERSPROC)(std::uint32_t, const char*, const char*, SNETCAPSPTR);
typedef void (STORMAPI* SNETEVENTPROC)(SNETEVENTPTR);
typedef BOOL(STORMAPI* SNETGETARTPROC)(std::uint32_t, std::uint32_t, LPPALETTEENTRY, LPBYTE, std::uint32_t, int*, int*, int*);
typedef BOOL(STORMAPI* SNETGETDATAPROC)(std::uint32_t, std::uint32_t, void*, std::uint32_t, std::uint32_t*);
typedef int (STORMAPI* SNETMESSAGEBOXPROC)(HWND, const char*, const char*, UINT);
typedef BOOL(STORMAPI* SNETPLAYSOUNDPROC)(std::uint32_t, std::uint32_t, std::uint32_t);
typedef BOOL(STORMAPI* SNETSELECTEDPROC)(std::uint32_t, SNETCAPSPTR, _SNETUIDATA*, _SNETVERSIONDATA*);
typedef BOOL(STORMAPI* SNETSTATUSPROC)(const char*, std::uint32_t, std::uint32_t, std::uint32_t, SNETABORTPROC);
typedef BOOL(STORMAPI* SNETPROFILEPROC)();
typedef BOOL(STORMAPI* SNETNEWACCOUNTPROC)();

typedef bool(__fastcall* CODEVERIFYPROC)(const char*);
typedef void(STORMAPI* SNETLEAGUECMDRESULTPROC)(const char*, int);


// TODO: identify profilecallback, profilefields, profilebitmapcallback
typedef struct _SNETUIDATA {
  std::uint32_t size;
  std::uint32_t uiflags;
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
  std::uint32_t                deviceid;
  SNETCAPS             devicecaps;
  char                 devicename[SNETSPI_MAXSTRINGLENGTH];
  char                 devicedescription[SNETSPI_MAXSTRINGLENGTH];
  std::uint32_t                reserved;
  _SNETSPI_DEVICELIST* next;
} SNETSPI_DEVICELIST, * SNETSPI_DEVICELISTPTR;

typedef struct _SNETSPI_GAMELIST {
  std::uint32_t              gameid;
  std::uint32_t              gamemode;
  std::uint32_t              creationtime;
  SNETADDR           owner;
  std::uint32_t              ownerlatency;
  std::uint32_t              ownerlasttime;
  std::uint32_t              gamecategorybits;
  char               gamename[SNETSPI_MAXSTRINGLENGTH];
  char               gamedescription[SNETSPI_MAXSTRINGLENGTH];
  _SNETSPI_GAMELIST* next;
  void*             clientdata;
  std::uint32_t              clientdatabytes;
  std::uint32_t              productid;
  std::uint32_t              version;
} SNETSPI_GAMELIST, * SNETSPI_GAMELISTPTR;

typedef struct _SNETSPI {
  // The size of the vtable
  std::uint32_t size;
  // Compares two sockaddrs with each other and returns the number of differences in dwResult
  BOOL(STORMAPI* spiCompareNetAddresses)(SNETADDRPTR addr1, SNETADDRPTR addr2, std::uint32_t* diffmagnitude);
  // Called when the module is released
  BOOL(STORMAPI* spiDestroy)();
  // Called in order to free blocks of packet memory returned in the spiReceive functions
  BOOL(STORMAPI* spiFree)(SNETADDRPTR addr, void* data, std::uint32_t databytes);
  BOOL(STORMAPI* spiFreeExternalMessage)(const char* addr, const char* data, const char* databytes); // TODO: possibly incorrect, investigate
  // Returns info on a specified game
  BOOL(STORMAPI* spiGetGameInfo)(std::uint32_t gameid, const char* gamename, const char* gamepassword, SNETSPI_GAMELIST* gameinfo);
  // Returns packet statistics
  BOOL(STORMAPI* spiGetPerformanceData)(std::uint32_t counterid, std::uint32_t* countervalue, LARGE_INTEGER* measurementtime, LARGE_INTEGER* measurementfreq);
  // Called when the module is initialized
  BOOL(STORMAPI* spiInitialize)(SNETPROGRAMDATAPTR programdata, SNETPLAYERDATAPTR playerdata, SNETUIDATAPTR interfacedata, SNETVERSIONDATAPTR versiondata, HANDLE hEvent);
  BOOL(STORMAPI* spiInitializeDevice)(std::uint32_t deviceid, SNETPROGRAMDATAPTR programdata, SNETPLAYERDATAPTR playerdata, SNETUIDATAPTR itnerfacedata, SNETVERSIONDATAPTR versiondata);
  BOOL(STORMAPI* spiLockDeviceList)(SNETSPI_DEVICELISTPTR* devicelist);
  // Called to prevent the game list from updating so that it can be processed by storm
  BOOL(STORMAPI* spiLockGameList)(std::uint32_t categorybits, std::uint32_t categorymask, SNETSPI_GAMELISTPTR* gamelist);
  // Return received data from a connectionless socket to storm
  BOOL(STORMAPI* spiReceive)(SNETADDRPTR* addr, void** data, std::uint32_t* databytes);
  // Return received data from a connected socket to storm
  BOOL(STORMAPI* spiReceiveExternalMessage)(SNETADDRPTR* addr, void** data, std::uint32_t* databytes); // TODO: possibly incorrect, investigate
  // Called when a game is selected to query information
  BOOL(STORMAPI* spiSelectGame)(std::uint32_t flags, SNETPROGRAMDATAPTR programdata, SNETPLAYERDATAPTR playerdata, SNETUIDATAPTR interfacedata, SNETVERSIONDATAPTR versiondata, std::uint32_t* playerid);
  // Sends data over a connectionless socket
  BOOL(STORMAPI* spiSend)(std::uint32_t addresses, SNETADDRPTR* addrlist, void* data, std::uint32_t databytes);
  // Sends data over a connected socket
  BOOL(STORMAPI* spiSendExternalMessage)(const char* senderpath, const char* sendername, const char* targetpath, const char* targetname, const char* message);
  // An extended version of spiStartAdvertisingGame
  // BOOL(STORMAPI* spiStartAdvertisingGame)(const char* gamename, const char* gamepassword, const char* gamedescription, std::uint32_t gamemode, std::uint32_t gameage, std::uint32_t gamecategorybits, std::uint32_t optcategorybits, LPCVOID clientdata, std::uint32_t clientdatabytes); <-- old
  BOOL(STORMAPI* spiStartAdvertisingGame)(const char* gamename, const char* gamepassword, const char* gamedescription, std::uint32_t gamemode, std::uint32_t gameage, std::uint32_t gamecategorybits, std::uint32_t optcategorybits, std::uint32_t, LPCVOID clientdata, std::uint32_t clientdatabytes);
  // Called to stop advertising the game
  BOOL(STORMAPI* spiStopAdvertisingGame)();
  BOOL(STORMAPI* spiUnlockDeviceList)(SNETSPI_DEVICELISTPTR devicelist);
  // Called after the game list has been processed and resume updating
  BOOL(STORMAPI* spiUnlockGameList)(SNETSPI_GAMELISTPTR gamelist, std::uint32_t* hintnextcall);
  BOOL(STORMAPI* spiGetLocalPlayerName)(const char* namebuffer, std::uint32_t namechars, const char* descbuffer, std::uint32_t descchars);
  BOOL(STORMAPI* spiReportGameResult)(std::uint32_t ladderid, std::uint32_t arraysize, const char* *namearray, std::uint32_t *resultarray, const char* textgameresult, const char* textplayerresult);
  BOOL(STORMAPI* spiCheckDataFile)(const char* filename, const void *data, std::uint32_t bytes, std::uint32_t *extendedresult);
  BOOL(STORMAPI* spiSendLeagueCommand)(const char* cmd, void* callback);
  BOOL(STORMAPI* spiSendReplayPath)(const char* replaypath, std::uint32_t gameid, const char* textgameresult);
  BOOL(STORMAPI* spiGetLeagueId)(std::uint32_t *pid);
  BOOL(STORMAPI* spiLeagueLogout)(const char* bnetName);
  // Retrieves the player name that last whispered you on battle.net
  BOOL(STORMAPI* spiGetReplyName)(const char* buffer, std::uint32_t buffersize);
} SNETSPI, *SNETSPIPTR;

typedef BOOL(STORMAPI* SNETSPIBIND)(std::uint32_t, SNETSPIPTR*);
typedef BOOL(STORMAPI* SNETSPIQUERY)(std::uint32_t, std::uint32_t*, const char**, const char**, SNETCAPSPTR*);


//###########################################################################
//## Functions
//###########################################################################

extern "C" {

  // @101
  BOOL STORMAPI SNetCreateGame(const char* gamename, const char* gamepassword, const char* gamedescription, std::uint32_t gamecategorybits, void* initdata, std::uint32_t initdatabytes, std::uint32_t maxplayers, const char* playername, const char* playerdescription, std::uint32_t* playerid);

  // @102
  BOOL STORMAPI SNetDestroy();

  // @103
  BOOL STORMAPI SNetEnumDevices(SNETENUMDEVICESPROC callback);

  // @105
  BOOL STORMAPI SNetEnumProviders(SNETCAPSPTR mincaps, SNETENUMPROVIDERSPROC callback);

  // @106
  BOOL STORMAPI SNetDropPlayer(std::uint32_t playerid, std::uint32_t exitcode);

  // @107
  BOOL STORMAPI SNetGetGameInfo(std::uint32_t index, void* buffer, std::uint32_t buffersize, std::uint32_t* byteswritten);

  // @109
  BOOL STORMAPI SNetGetNumPlayers(std::uint32_t* firstplayerid, std::uint32_t* lastplayerid, std::uint32_t* activeplayers);

  // @112
  BOOL STORMAPI SNetGetPlayerCaps(std::uint32_t playerid, SNETCAPSPTR caps);

  // @113
  BOOL STORMAPI SNetGetPlayerName(std::uint32_t playerid, char* buffer, std::uint32_t buffersize);

  // @114
  BOOL STORMAPI SNetGetProviderCaps(SNETCAPSPTR caps);

  // @115
  BOOL STORMAPI SNetGetTurnsInTransit(std::uint32_t* turns);

  // @116
  BOOL STORMAPI SNetInitializeDevice(std::uint32_t deviceid, SNETPROGRAMDATAPTR programdata, SNETPLAYERDATAPTR playerdata, SNETUIDATAPTR interfacedata, SNETVERSIONDATAPTR versiondata);

  // @117
  BOOL STORMAPI SNetInitializeProvider(std::uint32_t providerid, SNETPROGRAMDATAPTR programdata, SNETPLAYERDATAPTR playerdata, SNETUIDATAPTR interfacedata, SNETVERSIONDATAPTR versiondata);

  // @118
  BOOL STORMAPI SNetJoinGame(std::uint32_t gameid, const char* gamename, const char* gamepassword, const char* playername, const char* playerdescription, std::uint32_t* playerid);

  // @119
  BOOL STORMAPI SNetLeaveGame(std::uint32_t exitcode);

  // @120
  BOOL STORMAPI SNetPerformUpgrade(std::uint32_t* upgradestatus);

  // @121
  BOOL STORMAPI SNetReceiveMessage(std::uint32_t* senderplayerid, void** data, std::uint32_t* databytes);

  // @122
  BOOL STORMAPI SNetReceiveTurns(std::uint32_t firstplayerid, std::uint32_t arraysize, void** arraydata, std::uint32_t* arraydatabytes, std::uint32_t* arrayplayerstatus);

  // @123
  BOOL STORMAPI SNetRegisterEventHandler(std::uint32_t eventid, SNETEVENTPROC callback);

  // @124
  BOOL STORMAPI SNetResetLatencyMeasurements();

  // @125
  BOOL STORMAPI SNetSelectGame(std::uint32_t flags, SNETPROGRAMDATAPTR programdata, SNETPLAYERDATAPTR playerdata, SNETUIDATAPTR interfacedata, SNETVERSIONDATAPTR versiondata, std::uint32_t* playerid);

  // @127
  BOOL STORMAPI SNetSendMessage(std::uint32_t targetplayerid, void* data, std::uint32_t databytes);

  // @128
  BOOL STORMAPI SNetSendTurn(void* data, std::uint32_t databytes);

  // @129
  BOOL STORMAPI SNetSetBasePlayer(int playerid);

  // @130
  BOOL STORMAPI SNetSetGameMode(std::uint32_t modeflags);

  // @133
  BOOL STORMAPI SNetEnumGamesEx(std::uint32_t categorybits, std::uint32_t categorymask, SNETENUMGAMESEXPROC callback, std::uint32_t* hintnextcall);

  // @134
  BOOL STORMAPI SNetSendServerChatCommand(const char* command);

  // @137
  BOOL STORMAPI SNetDisconnectAll(std::uint32_t flags);

  // @138
  BOOL STORMAPI SNetCreateLadderGame(const char* gamename, const char* gamepassword, const char* gamedescription, std::uint32_t gamecategorybits, std::uint32_t ladderid, std::uint32_t gamemode, void* initdata, std::uint32_t initdatabytes, std::uint32_t maxplayers, const char* playername, const char* playerdescription, std::uint32_t* playerid);

  // @139
  BOOL STORMAPI SNetReportGameResult(unsigned firstplayerid, int arraysize, int* resultarray, const char* textgameresult, const char* textplayerresult);

  // @141
  BOOL STORMAPI SNetSendLeagueCommand(LPCTSTR cmd, SNETLEAGUECMDRESULTPROC callback);

  // @142
  int STORMAPI SNetSendReplayPath(const char* replaypath, std::uint32_t gameid, const char* textgameresult);

  // @143
  int STORMAPI SNetGetLeagueId(std::uint32_t* leagueID);

  // @144
  BOOL STORMAPI SNetGetPlayerNames(char** names);

  // @145
  int STORMAPI SNetLeagueLogout(char* bnetName);

  // @146
  int STORMAPI SNetGetReplyName(char* pszReplyName, size_t nameSize);

  // @147
  // Returns 4 byte protocol identifier of current protocol, only used for debugging, can be removed.
  std::uint32_t STORMAPI SNetGetCurrentProviderID();

  // @148
  void SNetSetCodeSignVerifcationFunction(CODEVERIFYPROC);
}

#endif
