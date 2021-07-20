#include "chessServer.h"
#include "wifi_usage.h"

char requestTemplate[150];
char requestBody[100];
char request[150];
char gameCode[50] = "";

void chessServer_init(uint32_t conntype)
{
    while (1)
    {
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
        if(connectionType == CLOSE_CONNECTION){
            disconnectFromAP();
            sl_Stop(0xFF);
        }

        sprintf(request, requestTemplate, "/ping");
        if (sendRequestToServer(request))
        {
            char parsedResponse[1024];
            parseServerResponse(parsedResponse, "pre-wrap\">");
            if (strstr(parsedResponse, "pong"))
            {
                printf("Connected to Server!\n");
                break;
            }
        }

        printf("Could not connect to server. Retrying.\n");
        DelayMs(3000);
    }
}

int8_t chessServer_makeMove(char *move)
{
    sprintf(requestBody, "/game/%s/makemove/%i/%s", gameCode, BOARD_ID, move);

    char parsedResponse[1024];
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

    char parsedResponse[1024];
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
    sprintf(requestBody, "/user/%i/newgame", BOARD_ID);

    char parsedResponse[1024];
    if (buildAndSendReq(parsedResponse))
    {
        strcpy(response, parsedResponse);
        char *pt = strstr(parsedResponse, "Id:");
        if (pt)
        {
            strncpy(gameCode, pt + 4, 20);
            return SUCCESS;
        }
        else
            return INVALID_RESPONSE;
    }
    else
        return REQUEST_FAILED;
}

int8_t chessServer_joinGame(char *response)
{
    sprintf(requestBody, "/user/%i/joingame/%s", BOARD_ID, gameCode);

    char parsedResponse[1024];
    if (buildAndSendReq(parsedResponse))
    {
        strcpy(response, parsedResponse);
        char *pt = strstr(parsedResponse, "game:");
        if (pt)
        {
            strncpy(gameCode, pt + 6, 20);
            return SUCCESS;
        }
        else
            return INVALID_RESPONSE;
    }
    else
        return REQUEST_FAILED;
}

int8_t chessServer_deleteGame(char *response)
{
    sprintf(requestBody, "/game/%s/delete", gameCode);

    char parsedResponse[1024];
    if (buildAndSendReq(parsedResponse))
    {
        strcpy(response, parsedResponse);
        if (strstr(parsedResponse, "Deleted"))
            return SUCCESS;
        else
            return INVALID_RESPONSE;
    }
    else
        return REQUEST_FAILED;
}

int8_t chessServer_awaitTurn()
{
    sprintf(requestBody, "/game/%s/turnready/%i", gameCode, BOARD_ID);

    char parsedResponse[1024];
    if (buildAndSendReq(parsedResponse))
    {
        if (strstr(parsedResponse, "Turn Ready"))
            return SUCCESS;
        else if (strstr(parsedResponse, "join"))
            return WAITING;
        else
            return INVALID_RESPONSE;
    }
    else
        return REQUEST_FAILED;
}

void chessServer_setGameCode(char *newGameCode)
{
    strcpy(gameCode, newGameCode);
}

int8_t chessServer_getFriends(char *response)
{
    sprintf(requestBody, "/user/%i/getfriends", BOARD_ID);

    char parsedResponse[1024];
    if (buildAndSendReq(parsedResponse))
    {
        //TODO: Process accordingly
        strcpy(response, parsedResponse);

        if (strstr(parsedResponse, "not exist"))
            return INVALID_RESPONSE;

        return SUCCESS;
    }
    else
        return REQUEST_FAILED;
}

int8_t chessServer_addFriend(char *response, uint16_t friendId)
{
    sprintf(requestBody, "/user/%i/addfriend/%i", BOARD_ID, friendId);

    char parsedResponse[1024];
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

int8_t chessServer_getCurrentGame()
{
    sprintf(requestBody, "/user/%i/getgame", BOARD_ID);

    char parsedResponse[1024];
    if (buildAndSendReq(parsedResponse))
    {
        char *pt = strstr(parsedResponse, "code:");
        if (pt)
        {
            strcpy(gameCode, pt + 6);
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

int8_t chessServer_setName(char* response, char* newName)
{
    sprintf(requestBody, "/user/%i/setname/%s", BOARD_ID, newName);

    char parsedResponse[1024];
    if (buildAndSendReq(parsedResponse))
    {
        strcpy(response, parsedResponse);
        if (strstr(parsedResponse, "succes"))
            return SUCCESS;
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
        parseServerResponse(parsedResponse, "pre-wrap\">");
        if (!parsedResponse)
            return 0;

        return 1;

    }
    else {
        restartWIFI();
        return 0;
    }
}
