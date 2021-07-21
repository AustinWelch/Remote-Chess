#include "boardLogicTest.h"

//TODO: Change all printf to LCD outputs
//      Change all scanf to button inputs

uint32_t mainMenu()
{
    while (1)
    {
        printf("Main Menu:\n> Game        > Friends\n> Settings\n");

        char response[50];

        scanf("%s", response);

        if (strstr(response, "ame"))
            return GAME;
        else if (strstr(response, "iends"))
            return FRIENDS;
        else
            return SETTINGS;
    }
}

uint32_t game()
{
    //If not in game ask to create game or join game
    int8_t resp;

    resp = chessServer_getCurrentGame();
    if (resp == REQUEST_FAILED || resp == INVALID_RESPONSE)
    {
        printf("Could not connect to server, trying again.\n");
        return GAME;
    }
    else if (resp == NOT_IN_GAME)
    {
        while (1)
        {
            char response[50];
            printf("Currently not in a game.\n> Create Game       > Join Game\n> Back\n");
            scanf("%s", response);
            if (strstr(response, "reate"))
            {
                char serverResponse[150];
                chessServer_newGame(serverResponse);
                printf(response);
                if(resp == INVALID_RESPONSE || resp == REQUEST_FAILED)
                    return GAME;
                break;
            }
            else if (strstr(response, "oin"))
            {
                while(1){
                    printf("Enter game code:\n");
                    scanf("%s", response);
                    chessServer_setGameCode(response);

                    char serverResponse[150];
                    resp = chessServer_joinGame(serverResponse);
                    printf(serverResponse);

                    if(resp == INVALID_RESPONSE)
                        continue;
                    else if(resp == REQUEST_FAILED){
                        return GAME;
                    }
                    break;
                }
                break;
            }
            else
                return MAIN_MENU;
        }
    }

    while(chessServer_awaitTurn() == WAITING){
        printf("Waiting for opponent to join.\n");
        DelayMs(3000);
    }

    //Main Game Loop
    while(1){
        
        char response[1024];
        printf("Turn: ");
        resp = chessServer_awaitTurn();

        if(resp == SUCCESS){
            
            printf("Yours\nMake a move (back to go back to menu):\n");
            //TODO: Dynamic move loading
            char posMov[1024];
            chessServer_getLegalMoves(posMov);
            printf(posMov);

            char move1[4], move2[4];
            while(1){
                printf("\nChoose a square:\n");
                scanf("%s", move1);
                if(strstr(move1, "ack"))
                    continue;

                printf("Move %s to:\n", move1);
                scanf("%s", move2);
                if(strstr(move2, "ack"))
                    continue;

                printf("Submit move %s%s?\n", move1, move2);
                scanf("%s", response);
                if(strstr(response, "es")){
                    char move[4];
                    sprintf(move, "%s%s", move1, move2);
                    chessServer_makeMove(move);
                    printf("Move Sent!\n");
                    break;
                }
                else
                    continue;
            }
            
        }
        else if(resp == INVALID_RESPONSE){
            printf("Opponent\n");
            DelayMs(5000);
        }

        else{
            printf("Server error, reconnecting\n");
            return GAME;
        }
    }
}

uint32_t settings()
{
    printf("Settings:\n> WIFI        > Board Functions\n> Back      > Change Name\n");

    char response[50];
    scanf("%s", response);

    if (strstr(response, "WIFI")){
        printf("Wifi Settings, press any key to leave\n");
        scanf("%s");
        return SETTINGS;
    }
    else if (strstr(response, "Board")){
        printf("Board Settings, press any key to leave\n");
        scanf("%s");
        return SETTINGS;
    }
    else if (strstr(response, "ame")){
        printf("> Change name       > Back\n");
        scanf("%s", response);
        if(strstr(response, "name")){
            printf("New name: \n");
            scanf("%s", response);

            int8_t resp;
            char serverResp[1024];
            resp = chessServer_setName(serverResp, response);
            printf(serverResp);
            return SETTINGS;
        }
        else
            return SETTINGS;
    }
        
    else
        return MAIN_MENU;
}

uint32_t friends()
{
    char friendsList[1024];
    chessServer_getFriends(friendsList);
    printf("%s\n> Add friend        > Invite friend\n> Back\n", friendsList);

    char response[50];
    scanf("%s", response);

    if (strstr(response, "Add")){
        printf("Add friend menu");
        char friendIdStr[32];
        char serverResp[1024];
        scanf("%s", friendIdStr);

        uint16_t friendId = strtol(friendIdStr, friendIdStr + 31, 20);

        chessServer_addFriend(serverResp, friendId);
        printf(serverResp);
        return FRIENDS;
    }
        
    else if (strstr(response, "Invite")){
        printf("Invite friend menu");
        scanf("%s");
        return FRIENDS;
    }
    
    else
        return MAIN_MENU;
}
