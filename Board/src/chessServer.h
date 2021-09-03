#ifndef CHESSSERVER_H_
#define CHESSSERVER_H_

#include "wifi_usage.h"
#include "BSP.h"
#include "rom.h"
#include "sysctl.h"
#include <stdio.h>
#include "application_commands.h"
#include <string.h>
#include "G8RTOS_Semaphores.h"

#define WEBPAGE "34.227.194.176"
// #define BOARD_ID 202
#define TO_STR(x) XTO_STR(x)
#define XTO_STR(x) #x

#define SUCCESS 0
#define REQUEST_FAILED -1
#define INVALID_RESPONSE -2
#define WAITING_FOR_JOIN -3
#define INVALID_MOVE -4
#define NOT_IN_GAME -5
#define NO_FRIENDS -6
#define NO_INVITES -7
#define WAITING_FOR_MOVE -8
#define SERVER_WON_GAME -9
#define SERVER_LOST_GAME -10
#define SERVER_OPP_RESIGNED -11

extern int boardID;
extern const char* boardIDStr;

extern Semaphore g_wifiSem;

typedef enum detail_ServerPlayer {
    LOCAL_MOVE, REMOTE_MOVE
} ServerPlayer;

typedef enum detail_PlayType {
    SERVER_ONLINE_PLAY, SERVER_LOCAL_PLAY
} ServerPlayType;

typedef struct detail_AwaitingMove {
    int8_t status;
    char algabreic[8];
    bool inCheck;
    char algabreicKingPosCheck[3];
    char algabreicKingPosWinner[3];
} AwaitingMove;

typedef struct detail_GameState {
    int8_t status;
    ServerPlayType playType;
    char algabreicLastMove[8];
    bool hasLastMove;
    ServerPlayer activePlayer;
    bool inCheck;
    char algabreicKingPosCheck[3];
    bool inCheckmate;
    char algabreicKingPosWinner[3];
} ServerGameState;

typedef struct detail_GameInvite {
    int8_t status;
    uint8_t playerId;
    char playerName[9];
    char gamecode[7];
} ServerGameInvite;

uint8_t chessServer_init(uint32_t conntype);
int8_t chessServer_makeMove(char *move, bool localP2);
int8_t chessServer_getLegalMoves(char* response);
int8_t chessServer_newGame(char* response);
int8_t chessServer_newGameCPU(char *response);
int8_t chessServer_newGameLocal();
int8_t chessServer_joinGame(char* response);
int8_t chessServer_resign();
ServerGameState chessServer_getGameState();
AwaitingMove chessServer_awaitTurn();
ServerGameInvite chessServer_getLastInvite();
void chessServer_setGameCode(char* newGameCode);
const char* chessServer_getGameCode();
int8_t chessServer_getFriends(char* response);
int8_t chessServer_addFriend(char* response, uint16_t friendId);
int8_t chessServer_cancelFriend(char* response, uint16_t friendId);
int8_t chessServer_acceptFriend(char* response, uint16_t friendId);
int8_t chessServer_declineFriend(char* response, uint16_t friendId);
int8_t chessServer_removeFriend(char* response, uint16_t friendId);
int8_t chessServer_getInvites(char* response);
int8_t chessServer_sendInvite(char* response, uint16_t inviteeId);
int8_t chessServer_cancelInvite(uint16_t inviteeId);
int8_t chessServer_acceptInvite(uint16_t inviterId);
int8_t chessServer_declineInvite(uint16_t inviterId);
int8_t chessServer_getCurrentGame();
int8_t chessServer_setName(char* response, char* newName);
int8_t chessServer_getName(char* response);
uint8_t chessServer_deleteGame();
uint8_t chessServer_leaveGame();
uint8_t buildAndSendReq(char *parsedResponse);

#endif
