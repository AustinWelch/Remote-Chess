#include "chessServer.h"
#include "wifi_usage.h"

Semaphore g_wifiSem = { 1 };


signed char SSID_NAME[100]   =    "BIGBALLER"   ;      /* Access point name to connect to. */
char PASSKEY[100]     =    "whatdoesthefoxsay";                  /* Password in case of secure AP */


char requestTemplate[150];
char requestBody[100];
char request[150];
char gameCode[50] = "";

char parsedResponse[1024];

int boardID;
const char* boardIDStr;

void SetBoardID(void) {
    if (TLV->TLV_CHECKSUM == 0xD770D947) {
        boardID = 201;
        boardIDStr = "201";
    } else if (TLV->TLV_CHECKSUM == 0x40EA5C08) {
        boardID = 202;
        boardIDStr = "202";
    }
}

uint8_t chessServer_init(uint32_t conntype)
{
    SetBoardID();

    int32_t retVal;

    sprintf(requestTemplate, "GET %s HTTP/1.1\r\nHost:%s\r\n\r\n", "%s", WEBPAGE);

    retVal = configureSimpleLinkToDefaultState();
    if (retVal < 0)
        printf("Error with SL configuration!");

    sl_Start(0, 0, 0);

    retVal = establishConnectionWithAP();
    if (retVal < 0)
        printf("Could not connect to AP!");

    sl_NetAppDnsGetHostByName((_i8 *)WEBPAGE, strlen(WEBPAGE), &DestinationIP, SL_AF_INET);

    connectionType = conntype;
    if (connectionType == CLOSE_CONNECTION)
    {
        disconnectFromAP();
        sl_Stop(0xFF);
    }

    sprintf(request, requestTemplate, "/ping");
    if (sendRequestToServer(request))
    {
        
        parseServerResponse(parsedResponse, "pre-wrap\">");
        if (strstr(parsedResponse, "pong"))
        {
            printf("Connected to Server!\n");
            return 1;
        }
    }

    printf("Could not connect to server. Retrying.\n");
    return 0;
}

int8_t chessServer_makeMove(char *move)
{
    sprintf(requestBody, "/game/%s/makemove/%i/%s", gameCode, boardID, move);

    
    if (buildAndSendReq(parsedResponse))
    {
        if (strstr(parsedResponse, "Success"))
            return SUCCESS;
        else if (strstr(parsedResponse, "not valid"))
            return INVALID_MOVE;
        else
            return INVALID_RESPONSE;
    }
    else
        return REQUEST_FAILED;
}

int8_t chessServer_getLegalMoves(char *response)
{
    sprintf(requestBody, "/game/%s/getlegalmoves", gameCode);

    
    if (buildAndSendReq(parsedResponse))
    {
        if (strstr(parsedResponse, "Legal"))
        {
            //Send response to be processed later
            strcpy(response, parsedResponse);
            return SUCCESS;
        }
        else
            return INVALID_RESPONSE;
    }
    else
        return REQUEST_FAILED;
}

int8_t chessServer_newGame(char *response)
{
    sprintf(requestBody, "/user/%i/newgame", boardID);

    if (buildAndSendReq(parsedResponse))
    {
        strcpy(response, parsedResponse);
        char *pt = strstr(parsedResponse, "Id:");
        if (pt)
        {
            strncpy(gameCode, pt + 4, 20);
            gameCode[6] = '\0';
            return SUCCESS;
        }
        else
            return INVALID_RESPONSE;
    }
    else
        return REQUEST_FAILED;
}

int8_t chessServer_newGameCPU(char *response)
{
    sprintf(requestBody, "/user/%i/newgamecpu", boardID);

    if (buildAndSendReq(parsedResponse))
    {
        strcpy(response, parsedResponse);
        char *pt = strstr(parsedResponse, "Id:");
        if (pt)
        {
            strncpy(gameCode, pt + 4, 20);
            gameCode[6] = '\0';
            return SUCCESS;
        }
        else
            return INVALID_RESPONSE;
    }
    else
        return REQUEST_FAILED;
}

ServerGameState chessServer_getGameState()
{
    ServerGameState retValue;

    sprintf(requestBody, "/game/%s/gamestate", gameCode);

    
    if (buildAndSendReq(parsedResponse))
    {
        const char* winner = strstr(parsedResponse, "Winner: W");

        if (winner) {
            char wonID[7];

            winner += 10;

            uint8_t size = strstr(winner, "!") - winner;
            strncpy(wonID, winner, size);
            wonID[size] = '\0';

            if (strcmp(wonID, boardIDStr) == 0) {
                retValue.status = SERVER_WON_GAME;
            } else {
                retValue.status = SERVER_LOST_GAME;
            }

            strncpy(retValue.algabreicKingPosWinner, strstr(winner, "!") + 1, 2);
            retValue.algabreicKingPosWinner[2] = '\0';

            strncpy(retValue.algabreicKingPosCheck, strstr(winner, "|") + 1, 2);
            retValue.algabreicKingPosCheck[2] = '\0';

            if (retValue.status == SERVER_WON_GAME) {
                return retValue;
            }
        } else if (strstr(parsedResponse, "Turn:") && strstr(parsedResponse, "Last Move"))
            retValue.status = SUCCESS;
        else
            retValue.status = INVALID_RESPONSE;
    }
    else
        retValue.status = REQUEST_FAILED;

    if (retValue.status == SUCCESS || retValue.status == SERVER_LOST_GAME) {
        // Get whose turn it currently is
        char* pt = strstr(parsedResponse, "Turn") + 9;
        int i = 0;
        char temp[7];
        while (*pt != ',') {
            temp[i] = *pt;
            pt++; i++;
        }

        int curTurnPlayer;
        sscanf(temp, "%d", &curTurnPlayer);

        if (curTurnPlayer == boardID) {
            retValue.activePlayer = LOCAL_MOVE; // If response matches our player id, it is our turn
        } else  {
            retValue.activePlayer = REMOTE_MOVE;
        }

        if (strstr(parsedResponse, "No previous move") == NULL) {
            // Has last move!
            char* algabreic = strstr(parsedResponse, "Last Move");
            algabreic += 11;

            strncpy(retValue.algabreicLastMove, algabreic, 5);
            retValue.algabreicLastMove[5] = '\0';

            retValue.hasLastMove = true;
        } else {
            retValue.hasLastMove = false;
        }

        char* check = strstr(parsedResponse, "In Check: Y");

        if (check) {
            retValue.inCheck = true;
            strncpy(retValue.algabreicKingPosCheck, strstr(check, "!") + 1, 2);
            retValue.algabreicKingPosCheck[2] = '\0';
        } else {
            retValue.inCheck = false;
        }
    }

    return retValue;
}

int8_t chessServer_joinGame(char *response)
{
    sprintf(requestBody, "/user/%i/joingame/%s", boardID, gameCode);

    
    if (buildAndSendReq(parsedResponse))
    {
        strcpy(response, parsedResponse);
        char *pt = strstr(parsedResponse, "game:");
        if (pt)
        {
            strncpy(gameCode, pt + 6, 6);
            gameCode[6] = '\0';
            return SUCCESS;
        }
        else
            return INVALID_RESPONSE;
    }
    else
        return REQUEST_FAILED;
}

int8_t chessServer_resign()
{
    sprintf(requestBody, "/game/%s/resign/%i", gameCode, boardID);

    if (buildAndSendReq(parsedResponse))
    {
        if (strstr(parsedResponse, "Resigned")) {
            return SUCCESS;
            chessServer_setGameCode("");
        }
        else
            return INVALID_RESPONSE;
    }
    else
        return REQUEST_FAILED;
}

AwaitingMove chessServer_awaitTurn()
{
    AwaitingMove retValue;

    sprintf(requestBody, "/game/%s/turnready/%i", gameCode, boardID);

    
    if (buildAndSendReq(parsedResponse))
    {
        const char* winner = strstr(parsedResponse, "Winner: W");

        if (winner) {
            char wonID[7];

            winner += 10;

            uint8_t size = strstr(winner, "!") - winner;
            strncpy(wonID, winner, size);
            wonID[size] = '\0';

            if (strcmp(wonID, boardIDStr) == 0) {
                retValue.status = SERVER_WON_GAME;
            } else {
                retValue.status = SERVER_LOST_GAME;
            }

            strncpy(retValue.algabreicKingPosWinner, strstr(winner, "!") + 1, 2);
            retValue.algabreicKingPosWinner[2] = '\0';

            strncpy(retValue.algabreicKingPosCheck, strstr(winner, "|") + 1, 2);
            retValue.algabreicKingPosCheck[2] = '\0';

            if (retValue.status == SERVER_WON_GAME) {
                return retValue;
            }
        } else if (strstr(parsedResponse, "resign"))
            retValue.status = SERVER_OPP_RESIGNED; 
        else if (strstr(parsedResponse, "Turn Ready"))
            retValue.status = SUCCESS;
        else if (strstr(parsedResponse, "Turn Not Ready"))
            retValue.status = WAITING_FOR_MOVE;
        else if (strstr(parsedResponse, "join"))
            retValue.status = WAITING_FOR_JOIN;
        else
            retValue.status = INVALID_RESPONSE;
    }
    else
        retValue.status = REQUEST_FAILED;

    if (retValue.status == SUCCESS || retValue.status == SERVER_LOST_GAME) {
        char* algabreic = strstr(parsedResponse, "Last Move");
        algabreic += 11;

        strncpy(retValue.algabreic, algabreic, 5);
        retValue.algabreic[5] = '\0';

        char* check = strstr(parsedResponse, "In Check: Y");

        if (check) {
            retValue.inCheck = true;
            strncpy(retValue.algabreicKingPosCheck, strstr(check, "!") + 1, 2);
            retValue.algabreicKingPosCheck[2] = '\0';
        } else {
            retValue.inCheck = false;
        }
    }

    return retValue;
}

void chessServer_setGameCode(char *newGameCode)
{
    strcpy(gameCode, newGameCode);
}

const char* chessServer_getGameCode() {
    return gameCode;
}

int8_t chessServer_getFriends(char *response)
{
    sprintf(requestBody, "/user/%i/getfriends", boardID);

    
    if (buildAndSendReq(parsedResponse))
    {
        strcpy(response, parsedResponse);

        if (strstr(parsedResponse, "not exist"))
            return INVALID_RESPONSE;

        else if (strstr(parsedResponse, "No Friends"))
            return NO_FRIENDS;

        return SUCCESS;
    }
    else
        return REQUEST_FAILED;
}

int8_t chessServer_addFriend(char *response, uint16_t friendId)
{
    sprintf(requestBody, "/user/%i/addfriend/%i", boardID, friendId);

    
    if (buildAndSendReq(parsedResponse))
    {
        strcpy(response, parsedResponse);
        if (strstr(parsedResponse, "Sent"))
            return SUCCESS;
        else
            return INVALID_RESPONSE;
    }
    else
        return REQUEST_FAILED;
}

int8_t chessServer_cancelFriend(char *response, uint16_t friendId)
{
    sprintf(requestBody, "/user/%i/cancelfriend/%i", boardID, friendId);

    
    if (buildAndSendReq(parsedResponse))
    {
        strcpy(response, parsedResponse);
        if (strstr(parsedResponse, "Canceled"))
            return SUCCESS;
        else
            return INVALID_RESPONSE;
    }
    else
        return REQUEST_FAILED;
}

int8_t chessServer_acceptFriend(char *response, uint16_t friendId)
{
    sprintf(requestBody, "/user/%i/acceptfriend/%i", boardID, friendId);

    
    if (buildAndSendReq(parsedResponse))
    {
        strcpy(response, parsedResponse);
        if (strstr(parsedResponse, "added"))
            return SUCCESS;
        else
            return INVALID_RESPONSE;
    }
    else
        return REQUEST_FAILED;
}

int8_t chessServer_declineFriend(char *response, uint16_t friendId)
{
    sprintf(requestBody, "/user/%i/declinefriend/%i", boardID, friendId);

    
    if (buildAndSendReq(parsedResponse))
    {
        strcpy(response, parsedResponse);
        if (strstr(parsedResponse, "Declined"))
            return SUCCESS;
        else
            return INVALID_RESPONSE;
    }
    else
        return REQUEST_FAILED;
}

int8_t chessServer_removeFriend(char *response, uint16_t friendId)
{
    sprintf(requestBody, "/user/%i/removefriend/%i", boardID, friendId);

    
    if (buildAndSendReq(parsedResponse))
    {
        strcpy(response, parsedResponse);
        if (strstr(parsedResponse, "Removed"))
            return SUCCESS;
        else
            return INVALID_RESPONSE;
    }
    else
        return REQUEST_FAILED;
}

int8_t chessServer_getInvites(char *response)
{
    sprintf(requestBody, "/user/%i/invites", boardID);

    
    if (buildAndSendReq(parsedResponse))
    {
        strcpy(response, parsedResponse);
        if (strstr(parsedResponse, "Invites"))
            return SUCCESS;
        else if (strstr(parsedResponse, "No invites"))
            return NO_INVITES;
        else 
            return INVALID_RESPONSE;
    }
    else
        return REQUEST_FAILED;
}

ServerGameInvite chessServer_getLastInvite()
{
    sprintf(requestBody, "/user/%i/getlastinvite", boardID);

    ServerGameInvite incomingInvite;

    if (buildAndSendReq(parsedResponse))
    {
        if (strstr(parsedResponse, "Last Invite")) {
            incomingInvite.status = SUCCESS;
            
            char* pt = parsedResponse + 13;
            int i;
        
            i = 0;
            char ID[7];
            while (*pt != ';'){
                ID[i] = *pt;
                pt++; i++;
            }
            ID[i] = '\0';

            int temp;
            sscanf(ID, "%d", &temp);
            incomingInvite.playerId = temp;
            pt++;

            i = 0;
            while (*pt != '!'){
                incomingInvite.playerName[i] = *pt;
                pt++; i++;
            }
            incomingInvite.playerName[i] = '\0';

            pt++;

            strcpy(incomingInvite.gamecode, pt);
        } else if (strstr(parsedResponse, "No invite"))
            incomingInvite.status = NO_INVITES;
        else 
            incomingInvite.status = INVALID_RESPONSE;
    }
    else
        incomingInvite.status = REQUEST_FAILED;

    return incomingInvite;
}

int8_t chessServer_sendInvite(char *response, uint16_t inviteeId)
{
    sprintf(requestBody, "/user/%i/sendinvite/%i", boardID, inviteeId);

    if (buildAndSendReq(parsedResponse))
    {
        strcpy(response, parsedResponse);
        if (strstr(parsedResponse, "success")) {
            char* pt = strstr(parsedResponse, "Code: ") + 6;

            char newGameCode[7];
            strcpy(newGameCode, pt);

            chessServer_setGameCode(newGameCode);

            return SUCCESS;
        }
        else
            return INVALID_RESPONSE;
    }
    else
        return REQUEST_FAILED;
}

int8_t chessServer_cancelInvite(uint16_t inviteeId)
{
    sprintf(requestBody, "/user/%i/cancelinvite/%i", boardID, inviteeId);

    
    if (buildAndSendReq(parsedResponse))
    {
        if (strstr(parsedResponse, "Canceled"))
            return SUCCESS;
        else
            return INVALID_RESPONSE;
    }
    else
        return REQUEST_FAILED;
}

int8_t chessServer_acceptInvite(uint16_t inviterId)
{
    sprintf(requestBody, "/user/%i/acceptinvite/%i", boardID, inviterId);

    if (buildAndSendReq(parsedResponse))
    {
        if (strstr(parsedResponse, "success"))
            return SUCCESS;
        else
            return INVALID_RESPONSE;
    }
    else
        return REQUEST_FAILED;
}

int8_t chessServer_declineInvite(uint16_t inviterId)
{
    sprintf(requestBody, "/user/%i/declineinvite/%i", boardID, inviterId);

    
    if (buildAndSendReq(parsedResponse))
    {
        if (strstr(parsedResponse, "declined"))
            return SUCCESS;
        else
            return INVALID_RESPONSE;
    }
    else
        return REQUEST_FAILED;
}

int8_t chessServer_getCurrentGame()
{
    sprintf(requestBody, "/user/%i/getgame", boardID);

    if (buildAndSendReq(parsedResponse))
    {
        // strcpy(response, parsedResponse);
        char *pt = strstr(parsedResponse, "code:");
        if (pt)
        {
            strncpy(gameCode, pt + 6, 6);
            gameCode[6] = '\0';

            return SUCCESS;
        }
        else if (strstr(parsedResponse, "not"))
            return NOT_IN_GAME;
        else
            return INVALID_RESPONSE;
    }
    else
        return REQUEST_FAILED;
}

int8_t chessServer_setName(char *response, char *newName)
{
    sprintf(requestBody, "/user/%i/setname/%s", boardID, newName);

    
    if (buildAndSendReq(parsedResponse))
    {
        strcpy(response, parsedResponse);
        if (strstr(parsedResponse, "success"))
            return SUCCESS;
        else
            return INVALID_RESPONSE;
    }
    else
        return REQUEST_FAILED;
}

int8_t chessServer_getName(char *response)
{
    sprintf(requestBody, "/user/%i/getname", boardID);

    
    if (buildAndSendReq(parsedResponse))
    {
        strcpy(response, parsedResponse);
        if (strstr(parsedResponse, "Name:"))
            return SUCCESS;
        else
            return INVALID_RESPONSE;
    }
    else
        return REQUEST_FAILED;
}

uint8_t chessServer_deleteGame() {  
    sprintf(requestBody, "/game/%s/delete", gameCode);

    if (buildAndSendReq(parsedResponse))
    {
        if (strstr(parsedResponse, "Deleted")) {
            memset(gameCode, 0, 7);
            return SUCCESS;
        }
        else
            return INVALID_RESPONSE;
    }
    else
        return REQUEST_FAILED;
}

uint8_t chessServer_leaveGame() {
    sprintf(requestBody, "/user/%s/leavegame", boardIDStr);

    if (buildAndSendReq(parsedResponse))
    {
        if (strstr(parsedResponse, "Left")) {
            memset(gameCode, 0, 7);
            return SUCCESS;
        }
        else
            return INVALID_RESPONSE;
    }
    else
        return REQUEST_FAILED;
}

uint8_t buildAndSendReq(char *parsedResponse)
{
    sprintf(request, requestTemplate, requestBody);
    if (sendRequestToServer(request))
    {
        memset(parsedResponse, '\0', 1024);

        parseServerResponse(parsedResponse, "pre-wrap\">");
        if (!parsedResponse)
            return 0;

        return 1;
    }
    else
    {
        restartWIFI();
        return 0;
    }
}
