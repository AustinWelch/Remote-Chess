#include "wifi_usage.h"
#include "BSP.h"
#include "debug.h"
#include "fpu.h"
#include "rom.h"
#include "sysctl.h"
#include <stdio.h>
#include "application_commands.h"
#include <string.h>

#define WEBPAGE "3.95.241.253"
#define BOARD_ID 0

#define SUCCESS 0
#define REQUEST_FAILED -1
#define INVALID_RESPONSE -2
#define WAITING -3
#define INVALID_MOVE -4
#define NOT_IN_GAME -5

void chessServer_init(uint32_t conntype);
int8_t chessServer_makeMove(char* move);
int8_t chessServer_getLegalMoves(char* response);
int8_t chessServer_newGame(char* response);
int8_t chessServer_joinGame(char* response);
int8_t chessServer_deleteGame(char* response);
int8_t chessServer_awaitTurn();
void chessServer_setGameCode(char* newGameCode);
int8_t chessServer_getFriends(char* response);
int8_t chessServer_addFriend(char* response, uint16_t friendId);
int8_t chessServer_getCurrentGame();
int8_t chessServer_setName(char* response, char* newName);
uint8_t buildAndSendReq(char *parsedResponse);
