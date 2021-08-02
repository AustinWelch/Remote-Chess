#ifndef CHESSSERVER_H_
#define CHESSSERVER_H_

#include "wifi_usage.h"
#include "BSP.h"
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
#define NO_FRIENDS -6
#define NO_INVITES -7

uint8_t chessServer_init(uint32_t conntype);
int8_t chessServer_makeMove(char* move);
int8_t chessServer_getLegalMoves(char* response);
int8_t chessServer_newGame(char* response);
int8_t chessServer_joinGame(char* response);
int8_t chessServer_deleteGame(char* response);
int8_t chessServer_awaitTurn(char* response);
void chessServer_setGameCode(char* newGameCode);
int8_t chessServer_getFriends(char* response);
int8_t chessServer_addFriend(char* response, uint16_t friendId);
int8_t chessServer_cancelFriend(char* response, uint16_t friendId);
int8_t chessServer_acceptFriend(char* response, uint16_t friendId);
int8_t chessServer_declineFriend(char* response, uint16_t friendId);
int8_t chessServer_removeFriend(char* response, uint16_t friendId);
int8_t chessServer_getInvites(char* response);
int8_t chessServer_sendInvite(char* response, uint16_t inviteeId);
int8_t chessServer_cancelInvite(char* response, uint16_t inviteeId);
int8_t chessServer_acceptInvite(char* response, uint16_t inviterId);
int8_t chessServer_declineInvite(char* response, uint16_t inviterId);
int8_t chessServer_getCurrentGame(char* response);
int8_t chessServer_setName(char* response, char* newName);
int8_t chessServer_getName(char* response);
uint8_t buildAndSendReq(char *parsedResponse);

#endif
