#include "chessServer.h"

char requestTemplate[150];

void chessServer_init()
{
    while(1){
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

        disconnectFromAP();
        sl_Stop(0xFF);

        char request[150];
        sprintf(request, requestTemplate, "/ping");
        if(sendRequestToServer(request) == 0){
            char parsedResponse[100];
            parseServerResponse(parsedResponse, "pre-wrap\">");
            if(strstr(parsedResponse, "pong")){
                printf("Connected to Server!")
                break;
            }
            else
                printf("Could not connect to server.")
        }

        DelayMs(3000);
    }
}

uint8_t chessServer_makeMove(char* gameCode, char* move)
{
    char moveRequest[100];
    char request[150];

    sprintf(moveRequest, "/game/%s/makemove/%i/%s", gameCode, BOARD_ID, move);
    sprintf(request, requestTemplate, moveRequest);
    if(sendRequestToServer(request) == 0){
        char parsedResponse[1000];
        parseServerResponse(parsedResponse, "pre-wrap\">");
        printf("%s\n", parsedResponse);
        if(strstr(parsedResponse, "Success"))
            return 1;
        else 
            return 0;
    }
    else{
        printf("Request to server failed!");
        return -1;
    }
}
