
#include "sl_usage.h"
#include "BSP.h"
#include "debug.h"
#include "fpu.h"
#include "rom.h"
#include "sysctl.h"
#include <stdio.h>
#include "application_commands.h"
#include <string.h>

char requestTemplate[150] = "GET %s HTTP/1.1\r\nHost:3.95.241.253\r\n\r\n";

/*
 * GLOBAL VARIABLES -- End
*/

#define WEBPAGE "3.95.241.253"
#define BOARD_ID 1

bool initialized = true;

int main(void)
{
    int32_t retVal;

    BSP_InitBoard();

    retVal = configureSimpleLinkToDefaultState();
    if (retVal < 0)
        printf("Error with SL configuration!");

    sl_Start(0, 0, 0);
    //Get IP from DNS
    retVal = establishConnectionWithAP();
    if (retVal < 0)
        printf("Could not connect to AP!");

    sl_NetAppDnsGetHostByName((_i8 *)WEBPAGE,
                                           strlen(WEBPAGE), &DestinationIP, SL_AF_INET);
    disconnectFromAP();
    sl_Stop(0xFF);

    printf("Welcome to Online Chess!\n");

    char request[100];
    sprintf(request, requestTemplate, "/game/MBSpINbKHQiwjTcEXoU4/getboard");
    if(sendRequestToServer(request) == 0){
        char parsedResponse[1000];
        parseServerResponse(parsedResponse, "pre-wrap");
        printf("%s\n", parsedResponse);
    }

    char moveTemplate[100] = "/game/MBSpINbKHQiwjTcEXoU4/makemove/%i/%s";

    while (1)
    {
        char input[100];
        printf("Make a move:\n");
        scanf("%s", input);

        char moveRequest[100];
        sprintf(moveRequest, moveTemplate, BOARD_ID, input);
        sprintf(request, requestTemplate, moveRequest);
        if(sendRequestToServer(request) == 0){
            char parsedResponse[1000];
            parseServerResponse(parsedResponse, "pre-wrap");
            printf("%s\n", parsedResponse);
        }
        else
            printf("Request to server failed!");

    }
}
