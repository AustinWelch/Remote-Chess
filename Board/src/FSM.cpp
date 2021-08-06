/*
    TODO:
    Finish up main FSM
        -processor try catch?
        -d e b u g
        -Put FSM code in switch statement?

    Board keyboard function

    Get piece placement w/ Lights

    Get Flash memory to work (if possible)

    Single Player vs Online AI

*/

#include "FSM.h"
#include "LCD_CharacterDisplay.h"
#include "Menu.h"

using namespace RemoteChess;

LCD_CharacterDisplay lcd;
Menu menu;

void FSM::FSMController() {
    while (true) {
        lcd.Clear();

        switch (curState) {
            case FSM::State::INITIAL_CONNECTION:
                FSM::InitialConnection();
                break;

            case FSM::State::INITIAL_WIFI_CHANGE:
                FSM::InitialWIFIChange();
                break;

            case FSM::State::MAIN_MENU:
                FSM::Main_Menu();
                break;

            case FSM::State::FRIENDS:
                FSM::Friends();
                break;

            case FSM::State::FRIENDS_ADD:
                FSM::FriendsAdd();
                break;

            case FSM::State::FRIENDS_SELECT:
                FSM::FriendsSelect();
                break;

            case FSM::State::FRIENDS_SELECT_INVITE:
                FSM::FriendsSelectInvite();
                break;

            case FSM::State::FRIENDS_SELECT_REMOVE:
                FSM::FriendsSelectRemove();
                break;

            case FSM::State::SETTINGS:
                FSM::Settings();
                break;

            case FSM::State::SETTINGS_BOARDPREFERENCES:
                FSM::SettingsBoardPreferences();
                break;

            case FSM::State::SETTINGS_WIFI:
                FSM::SettingsWifi();
                break;

            case FSM::State::FIND_GAME:
                FSM::FindGame();
                break;

            case FSM::State::WAITING_ON_P2:
                FSM::WaitingForPlayer();
                break;

            case FSM::State::JOIN_INVITE_CREATE:
                FSM::JoinInviteCreate();
                break;

            case FSM::State::CREATE:
                FSM::Create();
                break;

            case FSM::State::INVITE:
                FSM::Invite();
                break;

            case FSM::State::JOIN:
                FSM::Join();
                break;

            case FSM::State::INGAME:
                FSM::InGame();
                break;

            case FSM::State::INGAME_BOARDPREFERENCES:
                FSM::InGameBoardPreferences();
                break;

            case FSM::State::LEAVE_GAME:
                FSM::LeaveGame();
                break;

            default:
                break;
        }

        curState = nextState;
    }
}

//TODO: Set AP credentials in sl_common.h to those stored in flash
void FSM::InitialConnection() {
    int attempts = 0;
    while (attempts < 3) {
        if (chessServer_init(CLOSE_CONNECTION))
            break;

        attempts++;
        DelayMs(2000);
    }

    if (attempts == 3) { 
        lcd.WriteLineCentered("Failed to connect to server", 0);
        uint8_t resp = menu.DisplayMenuLeft(lcd, {"Retry", "Update WIFI Credentials", "", ""}, 1, 2);
        if (resp == 1) 
            nextState = FSM::State::INITIAL_CONNECTION;
        else
            nextState = FSM::State::INITIAL_WIFI_CHANGE;

        return;
    }

    nextState = FSM::State::MAIN_MENU;
}

void FSM::InitialWIFIChange() {
    lcd.WriteLine("Enter AP Name: ", 0);
    //char newAPName[100] = BoardKeyBoardFunction(); //Enter letters from sensors, print letters to LCD, middle button to submit

    lcd.WriteLine("Enter AP Pass: ", 1);
    
    //char newAPPass[100] = BoardKeyBoardFunction();

    //Save new credentials to flash

    //memcpy(SSID_NAME, newAPName, strlen(newAPName)); 
    //memcpy(PASSKEY, newAPPass, strlen(newAPPass));

    nextState = FSM::State::INITIAL_CONNECTION;
}

void FSM::Main_Menu() {
    char titleTemp[12] = "Welcome, %s";
    char title[20];
    char response[100];
    chessServer_getName(response);

    sprintf(title, titleTemp, response + 6);

    lcd.WriteLineCentered(title, 0);
    
    uint8_t buttonResp = menu.DisplayMenuLeft(lcd, {"Game", "Friends", "Settings", ""}, 1, 3);
    if (buttonResp == 1)
        nextState = FSM::State::FIND_GAME;

    if (buttonResp == 2)
        nextState = FSM::State::FRIENDS;

    if (buttonResp == 3)
        nextState = FSM::State::SETTINGS;
}

void FSM::Friends() {
    char response[1024];
    int retVal = chessServer_getFriends(response);
    parseFriends(response);
    
    if (retVal == INVALID_RESPONSE || retVal == REQUEST_FAILED) {
        lcd.WriteMessageWrapped(response);
        DelayMs(3000);
        nextState = FSM::State::FRIENDS; 
        return;
    }

    lcd.WriteLineCentered("Friends   ID: " + BOARD_ID , 0);    

    if (retVal == NO_FRIENDS) {
        lcd.WriteLineCentered("No friends added", 1);

        uint8_t buttonResponse = menu.DisplayMenuLeftRight(lcd, {"Add", "Back", "", "", "", "", "", ""}, 3, 2);

        if (buttonResponse == 6)
            nextState = FSM::State::FRIENDS_ADD;
        else 
            nextState = FSM::State::MAIN_MENU;

        return;
    }

    int8_t selection = menu.DisplayScrollingMenu(lcd, friends, friends.size(), "");

    if (selection == -2) {
        nextState = FSM::State::FRIENDS_ADD;
    } else if (selection == -1) {
        nextState = FSM::State::MAIN_MENU;
    } else {
        currentFriendID = friendIDs[selection];
        currentFriendName = friends[selection];
        nextState = FSM::State::FRIENDS_SELECT;
    }
}

void FSM::FriendsAdd() {
    while (true) {
        lcd.Clear();
        lcd.WriteLineCentered("Add Friends", 0);

        uint8_t buttonResponse = menu.DisplayMenuLeftRight(lcd, { "Send Request", "Incoming Requests", "Outgoing Requests", "Back", "", "", "", "" }, 1, 4);

        if (buttonResponse == 2) {
            lcd.Clear();
            lcd.WriteLine("Enter Friend's ID", 0);
            lcd.WriteLine("ID: ", 1);
            
            char friendID[100] = "101";//BoardKeyBoardFunction();

            //if submit
                char response[1024];
                int8_t serverResp = chessServer_addFriend(response, std::stoi(FSM::convertToString(friendID, strlen(friendID))));
                lcd.Clear();
                lcd.WriteMessageWrapped(response);
                DelayMs(3000);
                if (serverResp == INVALID_RESPONSE || serverResp == REQUEST_FAILED)
                    nextState = FSM::State::FRIENDS_ADD;
                    return;

            //if back
                continue;
        }

        else if (buttonResponse == 3) {
            while (true) {
                lcd.Clear();
                lcd.WriteLineCentered("Incoming Requests", 0);
                
                int8_t selection = menu.DisplayScrollingMenu(lcd, incoming_friends, incoming_friends.size(), "");

                if (selection > 0) {
                    lcd.Clear();
                    char name[20];
                    convertToChar(incoming_friends[selection], name);
                    lcd.WriteLineCentered(name, 0);
                    
                    buttonResponse = menu.DisplayMenuLeft(lcd, {"Accept", "Decline", "Back", ""}, 1, 3);

                    char serverResponse[1024];
                    if (buttonResponse == 1) {
                        lcd.Clear();
                        chessServer_acceptFriend(serverResponse, incoming_friendIDs[selection]);

                        incoming_friends.erase(incoming_friends[selection]);

                        lcd.WriteMessageWrapped(serverResponse);
                        DelayMs(3000);
                    }
                    else if (buttonResponse == 2) {
                        lcd.Clear();
                        chessServer_declineFriend(serverResponse, incoming_friendIDs[selection]);

                        incoming_friends.erase(incoming_friends[selection]);

                        lcd.WriteMessageWrapped(serverResponse);
                        DelayMs(3000);
                    }
                    else
                        continue;
                }
                else
                    break;
            }
        }

        else if (buttonResponse == 4) {
            while (true) {
                lcd.Clear();
                lcd.WriteLineCentered("Outgoing Requests", 0);
                
                int8_t selection = menu.DisplayScrollingMenu(lcd, outgoing_friends, outgoing_friends.size(), "");

                if (selection > 0) {
                    lcd.Clear();
                    char name[20];
                    convertToChar(outgoing_friends[selection], name);
                    lcd.WriteLineCentered(name, 0);
                    
                    buttonResponse = menu.DisplayMenuLeft(lcd, {"Cancel", "Back", "", ""}, 1, 2);

                    char serverResponse[1024];
                    if (buttonResponse == 1) {
                        lcd.Clear();
                        chessServer_cancelFriend(serverResponse, outgoing_friendIDs[selection]);

                        outgoing_friends.erase(outgoing_friends[selection]);

                        lcd.WriteMessageWrapped(serverResponse);
                        DelayMs(3000);
                    }
                    else {
                        continue;
                    }
                }
                else
                    break;
            }
        }

        else
            break;
    }
    
    nextState = FSM::State::FRIENDS;
}

void FSM::FriendsSelect() {
    char name[20];
    convertToChar(currentFriendName, name);
    lcd.WriteLineCentered(name, 0);
    
    uint8_t buttonResponse = menu.DisplayMenuLeft(lcd, {"Invite to Game", "Remove Friend", "Back", ""}, 1, 3);
  
    if (buttonResponse == 1)
        nextState = FSM::State::FRIENDS_SELECT_INVITE;

    else if (buttonResponse == 2)
        nextState = FSM::State::FRIENDS_SELECT_REMOVE;

    else
        nextState = FSM::State::FRIENDS;
}

void FSM::FriendsSelectInvite() {
    char response[1024];
    int8_t retVal = chessServer_sendInvite(response, currentFriendID);
    lcd.WriteMessageWrapped(response);

    DelayMs(3000);

    if (retVal == SUCCESS) {
        lcd.Clear();
        char name[20];
        lcd.WriteLineCentered("Waiting for", 0);
        FSM::convertToChar(currentFriendName + "to join", name);
        lcd.WriteLineCentered(name, 1);
        
        lcd.WriteLineCentered("Cancel - Any button", 3);

        P8->IFG = 0;
        P8->IE |= BIT3 | BIT4 | BIT5 | BIT6 | BIT7;

        bool buttonPressed = false;

        while (true) {
            for (uint32_t i = 0; i < 30000000; i++){
                if(menu.getButtonInput() != 0) {
                    buttonPressed = true;
                    break;
                }
            }

            if (buttonPressed) {
                char response[64];
                retVal = chessServer_cancelInvite(response, currentFriendID);
                nextState = FSM::State::FRIENDS_SELECT;
                return;
            }

            retVal = chessServer_awaitTurn(response);

            if (retVal == SUCCESS) {
                joiningAsColor = PlayerColor::WHITE;
                nextState = FSM::State::INGAME;
                return;
            }
        }
    } else {
        nextState = FSM::State::FRIENDS_SELECT;
    }
}

void FSM::FriendsSelectRemove() {
    lcd.WriteLineCentered("Are you sure you", 0);
    lcd.WriteLineCentered("want to to remove", 1);
    char name[20];
    FSM::convertToChar(currentFriendName + "?", name);
    lcd.WriteLineCentered(name, 2);
    
    uint8_t buttonResponse = menu.DisplayMenuLeftRight(lcd, {"Yes", "No", "", "", "", "", "", ""}, 3, 2);
    if (buttonResponse == 1) {
        char response[128];
        int8_t retVal = chessServer_removeFriend(response, currentFriendID);
        lcd.WriteMessageWrapped(response);
        DelayMs(1500);
    }

    nextState = FSM::State::FRIENDS;
}

void FSM::Settings() {
    lcd.WriteLineCentered("Settings", 0);
    uint8_t buttonResponse = menu.DisplayMenuLeftRight(lcd, {"Board Pref", "WIFI", "Name", "Back", "", "", "", ""}, 1, 4);
   
    if (buttonResponse == 2) {
        nextState = FSM::State::SETTINGS_BOARDPREFERENCES;
    } else if (buttonResponse == 3) {
        nextState = FSM::State::SETTINGS_WIFI;
    } else if (buttonResponse == 4) {
        nextState = FSM::State::SETTINGS_NAMECHANGE;
    } else {
        nextState = FSM::State::MAIN_MENU;
    }
}

void FSM::SettingsNameChange() {
    lcd.WriteLineCentered("Enter new name: ", 1);
    
    char newName[100];
    uint8_t retVal = 0;//BoardKeyBoardFunction(); //Enter letters from sensors, print letters to LCD, middle button to submit

    if (retVal == 1) {
        char response[1024];
        chessServer_setName(response, newName);
        lcd.WriteMessageWrapped(response);
        DelayMs(3000);
    }

    nextState = FSM::State::SETTINGS;
}

void FSM::SettingsBoardPreferences() {
    //TODO: initialize variables in ChessBoard for lights and sound, load those settings form flash and just change runtime and write here
    //Write current setting next to menu selection 

    while(true){
        lcd.WriteLineCentered("Board Preferences", 0);
        
        uint8_t buttonResponse = menu.DisplayMenuLeft(lcd, {"Assist Lights:", "Sound:", "Back", ""}, 1, 3);

        if (buttonResponse == 1) {
            //usingLights = usingLights;
            //save settings to flash or db
        } else if (buttonResponse == 2) {
            //usingSound = !usingSound;
            //save settings to flash or db
        } else {
            break;
        }
    }

    nextState = FSM::State::SETTINGS;
}

void FSM::SettingsWifi() {
    nextState = FSM::State::SETTINGS;

    lcd.WriteLine("Enter AP Name:", 0);
    
    char newAPName[100]; 
    uint8_t retVal = 0;//BoardKeyBoardFunction(newAPName); //Enter letters from sensors, print letters to LCD, middle button to submit

    if (retVal == 0) {
        return;
    }
    
    lcd.WriteLine("Enter AP Pass:", 2);
    
    char newAPPass[100]; 
    retVal = 0;//BoardKeyBoardFunction(newAPPass);

    if (retVal == 0) {
        return;
    }

    //Save new credentials to flash

    //memcpy(SSID_NAME, newAPName, strlen(newAPName)); 
    //memcpy(PASSKEY, newAPPass, strlen(newAPPass));
}

void FSM::FindGame() {
    char response[1024];
    int resp = chessServer_getCurrentGame(response);
    if (resp == SUCCESS) {
        nextState = FSM::State::INGAME;
        
        if(strstr(response, "WHITE"))
            joiningAsColor = PlayerColor::WHITE;
        else
            joiningAsColor = PlayerColor::BLACK;

        return;
    }
    else if (resp == NOT_IN_GAME)
        nextState = FSM::State::JOIN_INVITE_CREATE;
    else if (resp == WAITING) 
        nextState = FSM::State::WAITING_ON_P2;
    else
        nextState = FSM::State::MAIN_MENU;

    lcd.WriteMessageWrapped(response);
    DelayMs(1500);
}

void FSM::WaitingForPlayer() { 
    lcd.WriteLineCentered("Waiting for", 0);
    lcd.WriteLineCentered("opponent to join", 1);
    lcd.WriteLineCentered("Cancel         Back", 3);

    P8->IFG = 0;
    P8->IE |= BIT3 | BIT4 | BIT5 | BIT6 | BIT7;

    bool buttonPressed = false;

    char response[1024];
    while (true) {
        for (uint32_t i = 0; i < 30000000; i++){
            if(menu.getButtonInput() != 0) {
                buttonPressed = true;
                break;
            }
        }

        if (buttonPressed) {
            if(menu.getButtonInput() == BIT4) {
                    nextState = FSM::State::MAIN_MENU;
            } else if (menu.getButtonInput() == BIT6) {
                chessServer_deleteGame(response);
                lcd.WriteMessageWrapped(response);
                DelayMs(1500);
                nextState = FSM::State::JOIN_INVITE_CREATE;
                return;
            } else {
                buttonPressed = false;
            }
        }

        int8_t retVal = chessServer_awaitTurn(response);

        if (retVal == SUCCESS) {
            FSM::joiningAsColor = PlayerColor::WHITE;
            nextState = FSM::State::INGAME;
            return;
        }
    }

}

void FSM::JoinInviteCreate() {
    uint8_t buttonResponse = menu.DisplayMenuLeft(lcd, { "Join a game", "Invite friend", "Create a game", "Back" }, 0, 4);

    if (buttonResponse == 0) {
        nextState = FSM::State::JOIN;
    } else if (buttonResponse == 1) {
        nextState = FSM::State::INVITE;
    } else if (buttonResponse == 2) {
        nextState = FSM::State::CREATE;
    } else {
        nextState = FSM::State::MAIN_MENU;        
    }
}

void FSM::Create() {
    char response[1024];
    int8_t retVal = chessServer_newGame(response);
    lcd.WriteMessageWrapped(response);
    DelayMs(1500);
    if (retVal == SUCCESS) {
        lcd.WriteLineCentered("Waiting for", 0);
        lcd.WriteLineCentered("opponent to join", 1);
        lcd.WriteLineCentered("Main Menu    Cancel", 3);
        
        P8->IFG = 0;
        P8->IE |= BIT3 | BIT4 | BIT5 | BIT6 | BIT7;

        bool buttonPressed = false;

        while (true) {
            for (uint32_t i = 0; i < 30000000; i++){
                if(menu.getButtonInput() != 0) {
                    buttonPressed = true;
                    break;
                }
            }

            if (buttonPressed) {
                if(menu.getButtonInput() == BIT4) {
                    nextState = FSM::State::MAIN_MENU;
                } else if (menu.getButtonInput() == BIT6) {
                    chessServer_deleteGame(response);
                    lcd.WriteMessageWrapped(response);
                    DelayMs(1500);
                    nextState = FSM::State::JOIN_INVITE_CREATE;
                    return;
                } else {
                    buttonPressed = false;
                }
            }

            int8_t retVal = chessServer_awaitTurn(response);

            if (retVal == SUCCESS) {
                FSM::joiningAsColor = PlayerColor::WHITE;
                nextState = FSM::State::INGAME;
                return;
            }
        }

    } else {
        nextState = FSM::State::JOIN_INVITE_CREATE;
    }
}

void FSM::Invite() {
    char response[1024];
    int retVal = chessServer_getFriends(response);
    
    if (retVal == INVALID_RESPONSE || retVal == REQUEST_FAILED) {
        nextState = FSM::State::FRIENDS; 
        return;
    }

    lcd.WriteLineCentered("Invite Friends", 0);
    
    if (retVal == NO_FRIENDS) {
        lcd.WriteLineCentered("You currently have no friends to invite", 1);
        
        menu.DisplayMenuLeft(lcd, {"Back", "", "", ""}, 3, 1);
        
        nextState = FSM::State::JOIN_INVITE_CREATE;
    }

    parseFriends(response);

    while(true) {
        lcd.Clear();

        int8_t selection = menu.DisplayScrollingMenu(lcd, friends, friends.size(), " ");

        currentFriendName = friends[selection];
        currentFriendID = friendIDs[selection];

        char name[20];
        FSM::convertToChar("Invite " + currentFriendName + "?", name);

        if (selection > 0) {
            lcd.Clear();
            lcd.WriteLineCentered(name, 0);
            
            uint8_t buttonResponse = menu.DisplayMenuLeftRight(lcd, { "Yes", "No", "", "", "", "", "", "" }, 2, 2);

            if (buttonResponse == 1) {
                retVal = chessServer_sendInvite(response, currentFriendID);
                lcd.WriteMessageWrapped(response);
                DelayMs(1500);

                if (retVal == SUCCESS) {
                    lcd.WriteLineCentered("Waiting for", 0);
                    FSM::convertToChar(currentFriendName + "to join", name);
                    lcd.WriteLineCentered(name, 1);
                    
                    lcd.WriteLineCentered("Cancel - Any button", 3);

                    P8->IFG = 0;
                    P8->IE |= BIT3 | BIT4 | BIT5 | BIT6 | BIT7;

                    bool buttonPressed = false;

                    while (true) {
                        for (uint32_t i = 0; i < 30000000; i++){
                            if(menu.getButtonInput() != 0) {
                                buttonPressed = true;
                                break;
                            }
                        }

                        if (buttonPressed) {
                            char response[64];
                            retVal = chessServer_cancelInvite(response, currentFriendID);
                            nextState = FSM::State::INVITE;
                            return;
                        }

                        retVal = chessServer_awaitTurn(response);

                        if (retVal == SUCCESS) {
                            FSM::joiningAsColor = PlayerColor::WHITE;
                            nextState = FSM::State::INGAME;
                            return;
                        }
                    }
                }
            } else {
                continue;
            }
        } else { 
            break;
        }
    }
    nextState = FSM::State::JOIN_INVITE_CREATE;
}

void FSM::Join() {
    while (true) {
        lcd.WriteLineCentered("Join a Game", 0);
        
        uint8_t buttonResponse = menu.DisplayMenuLeft(lcd, { "Invites", "Game Code", "Back", "" }, 1, 3);
  
        if (buttonResponse == 1) {
            lcd.Clear();
            lcd.WriteLineCentered("Invites", 0);
            
            while (true) {
                char response[1024];
                int8_t retVal = chessServer_getInvites(response);

                if (retVal == INVALID_RESPONSE || retVal == REQUEST_FAILED) {
                    lcd.WriteMessageWrapped(response);
                    DelayMs(1500);
                }

                else if (retVal != NO_INVITES) {
                    parseInvites(response);
                    int8_t selection = menu.DisplayScrollingMenu(lcd, inviterNames, inviterNames.size(), "");

                    if (selection > 0) {
                        lcd.Clear();
                        char name[20];
                        FSM::convertToChar("Invite " + inviterNames[selection] + "?", name);
                        lcd.WriteLineCentered(name, 0);
                            
                        uint8_t buttonResponse = menu.DisplayMenuLeftRight(lcd, { "Yes", "No", "Back", "", "", "", "", "" }, 2, 3);

                        if (buttonResponse == 1) {
                            retVal = chessServer_acceptInvite(response, inviterIDs[selection]);
                            lcd.WriteMessageWrapped(response);
                            DelayMs(1500);
                            if (retVal == SUCCESS) {
                                FSM::joiningAsColor = PlayerColor::BLACK;
                                nextState = FSM::State::INGAME;
                                return;
                            } else {
                                continue;
                            }

                        } else if (buttonResponse == 2) {
                            retVal = chessServer_declineInvite(response, inviterIDs[selection]);
                            lcd.WriteMessageWrapped(response);
                            DelayMs(1500);
                            continue;

                        } else {
                            continue;
                        }
                    } else {
                        break;
                    }
                } else { 
                    lcd.WriteLineCentered("No friends added", 1);
                    menu.DisplayMenuLeft(lcd, { "Back", "", "", "" }, 3, 1);
                    break;
                }
            }
        } else if (buttonResponse == 2) {
            lcd.Clear();
            lcd.WriteLineCentered("Enter Game Code:", 1);
            
            char gameCode[10];
            uint8_t retVal = 0;//BoardKeyBoardFunction(gameCode); //Enter characters from sensors, print characters to LCD, middle button to submit
           
            if (retVal == 1) {
                char response[1024];
                chessServer_setGameCode(gameCode);
                retVal = chessServer_joinGame(response);

                lcd.WriteMessageWrapped(response);
                DelayMs(1500);

                if(retVal == SUCCESS) {
                    joiningAsColor = PlayerColor::BLACK;
                    nextState = FSM::State::INGAME;
                    return;
                } else {
                    continue;
                }
            } else {
                continue;
            }
        } else {
            break;
        }
    }

    nextState = FSM::State::JOIN_INVITE_CREATE;
}

void FSM::InGame() {
    //-If player's turn, check opponent's move, if move was followed through, initialize boardFSM with awaitLocalMove
    // else, initialize boardFSM with awaitFollowThorugh
    //-If opponent's turn initialize boardFSM with awaitMoveNotice
    
    ChessBoard::BoardState initialBoardState;
    bool triggerIncomingMove = false;
    Cell from, to;

    while (true) {
        char response[1024];
        int8_t serverResp = chessServer_awaitTurn(response);

        if (serverResp == SUCCESS) {
            char *pt = response + 15; //TODO:change number
            from = Cell(*(pt + 1) - 96, *(pt + 2) - 48);
            to = Cell(*(pt + 3) - 96, *(pt + 4) - 48);

            //if (magnets_isPieceAt(from)){
                initialBoardState = ChessBoard::BoardState::AWAITING_REMOTE_MOVE_NOTICE;
                triggerIncomingMove = true;
            //}

            //else
                initialBoardState = ChessBoard::BoardState::AWAITING_LOCAL_MOVE;

            break;
        }

        else if (serverResp == WAITING) {
            initialBoardState = ChessBoard::BoardState::AWAITING_REMOTE_MOVE_NOTICE;
            break;
        }

        else 
            continue;
    }

    ChessBoard gameBoard(joiningAsColor, initialBoardState);

    if (triggerIncomingMove)
        gameBoard.ReceiveRemoteMove(Move(from, to));
    
    while (true) {
        lcd.Clear();
        if (gameBoard.GetBoardState() == ChessBoard::BoardState::AWAITING_REMOTE_MOVE_NOTICE) {
            while (!turnReady) {
                lcd.WriteLineCentered("Waiting for", 0);
                lcd.WriteLineCentered("opponent's move...", 1);
                lcd.WriteLineCentered("Board Pref.   Leave", 3);

                P8->IFG = 0;
                P8->IE |= BIT3 | BIT4 | BIT5 | BIT6 | BIT7;

                bool buttonPressed = false;

                while (true) {
                    for (uint32_t i = 0; i < 30000000; i++){
                        if(menu.getButtonInput() != 0) {
                            buttonPressed = true;
                            break;
                        }
                    }

                    if (buttonPressed) {
                        if(menu.getButtonInput() == BIT4) {
                            nextState = State::INGAME_BOARDPREFERENCES;
                            return;
                        } else if (menu.getButtonInput() == BIT6) {
                            nextState = State::LEAVE_GAME;
                            return;
                        } else {
                            buttonPressed = false;
                        }
                    }
                    char response[64];
                    int8_t retVal = chessServer_awaitTurn(response);

                    if (retVal == SUCCESS) {
                       break;
                    }
                }
            }
            continue;
        }

        else if (gameBoard.GetBoardState() == ChessBoard::BoardState::AWAITING_LOCAL_MOVE) {
            std::string liftedPieceName = "";
            while (true) {
                lcd.Clear();
                lcd.WriteLineCentered("Your Turn", 0);
                
                liftedPieceName = gameBoard.GetLiftedPieceName();

                char movePieceName[20];
                if (liftedPieceName.size() == 0) {
                    strcpy(movePieceName, "No piece chosen");
                } else if (liftedPieceName.size() <= 1) {
                    strcpy(movePieceName, "Cannot move piece");
                } else {    
                    convertToChar("Move " + liftedPieceName, movePieceName);
                }
                //TODO: dynamically change piecename
                uint8_t buttonResponse = menu.DisplayMenuLeft(lcd, { movePieceName, "Board Preferences", "Back", "" }, 1, 3);
                
                if (buttonResponse == 1 && liftedPieceName.size() != 0) {
                    lcd.Clear();

                    Cell lift = gameBoard.GetLiftedPiecePos();
                    Cell place = gameBoard.GetPlacedPiecePos();

                    if (gameBoard.GetInvalidLifts().contains(lift)) {
                        lcd.WriteLineCentered("Mutliple pieces", 0);
                        lcd.WriteLineCentered("lifted", 1);
                        continue;
                    } else if (gameBoard.GetInvalidPlacements().contains(place)) {
                        lcd.WriteLineCentered("Invalid placement", 0);
                        continue;
                    }
                    
                    lcd.WriteLineCentered("Would you like to ", 0);
                    char name[20];
                    convertToChar("move your " + liftedPieceName + "?", name);
                    lcd.WriteLineCentered(name, 1);

                    uint8_t buttonResponse = menu.DisplayMenuLeftRight(lcd, { "Yes", "No", "", "", "", "", "", "" }, 3, 2);
                    
                    if (buttonResponse == 1) {
                        char move[4];
                        move[0] = lift.file + 96;
                        move[1] = lift.rank + 48;
                        move[2] = place.file + 96;
                        move[3] = place.rank + 48;

                        int8_t retVal = chessServer_makeMove(move);

                        if (retVal == SUCCESS) {
                            lcd.Clear();
                            lcd.WriteLineCentered("Move Sent!", 1);
                            gameBoard.SubmitCurrentLocalMove();
                        } else {
                            lcd.Clear();
                            lcd.WriteMessageWrapped("Something went wrong, try again");
                        }
                        DelayMs(1500);
                        continue;
                    } else {
                        continue;
                    }
                } else if (buttonResponse == 2) {
                    nextState = State::INGAME_BOARDPREFERENCES;
                    return;
                } else if (buttonResponse == 3) {
                    nextState = State::LEAVE_GAME;
                    return;
                }
            }
        }

        else if (gameBoard.GetBoardState() == ChessBoard::BoardState::AWAITING_REMOTE_MOVE_FOLLOWTHROUGH) {
            lcd.WriteLineCentered("Opponent moved", 0);
            char name[20];
            convertToChar("their " , name);
            lcd.WriteLineCentered(name, 1);
            //TODO: Get last moved piece name
            while(gameBoard.GetBoardState() == ChessBoard::BoardState::AWAITING_REMOTE_MOVE_FOLLOWTHROUGH);
        }
    }
}

void FSM::InGameBoardPreferences() {
    //TODO: initialize variables in ChessBoard for lights and sound, load those settings form flash and just change runtime and write here
    //Write current setting next to menu selection 

    while(true){
        lcd.WriteLineCentered("Board Preferences", 0);
        
        uint8_t buttonResponse = menu.DisplayMenuLeft(lcd, {"Assist Lights:", "Sound:", "Back", ""}, 1, 3);

        if (buttonResponse == 1) {
            //usingLights = usingLights;
            //save settings to flash or db
        } else if (buttonResponse == 2) {
            //usingSound = !usingSound;
            //save settings to flash or db
        } else {
            break;
        }
    }

    nextState = FSM::State::INGAME;
}

void FSM::LeaveGame() {
    lcd.WriteLineCentered("Leave Game?", 0);
    
    uint8_t buttonResponse = menu.DisplayMenuLeft(lcd, { "Yes", "No", "", "" }, 2, 2);
   
    if (buttonResponse == 2)
        lcd.Clear();
        lcd.WriteLineCentered("Leaving Game...", 0);
        nextState = FSM::State::MAIN_MENU;
        while (true) {
            char response[64];
            int8_t retVal = chessServer_deleteGame(response);
            if (retVal == SUCCESS) {
                return;
            } else {
                continue;
            }
        }
}


// HELPER FUNCTIONS

std::string FSM::convertToString(char* ch_a, int length) {
    std::string retString = "";

    for (int i = 0; i < length; i++)
        retString += ch_a[i];

    return retString;    
}

void FSM::convertToChar(std::string str, char* out) {
    for (int i = 0; i < str.size(); i++) {
        out[i] = str[i];
    }
}

void FSM::parseFriends(char* response) {
    FSM::friends.erase_all();
    FSM::friendIDs.erase_all();
    FSM::incoming_friends.erase_all();
    FSM::incoming_friendIDs.erase_all();
    FSM::outgoing_friends.erase_all();
    FSM::outgoing_friendIDs.erase_all();

    char* pt = response + 9;

    while (*pt != ';') {
        if(*pt == 'N'){
            pt += 10;
            break;
        }

        std::string ID = "";
        while (*pt != ','){
            ID += *pt;
            pt++; 
        }
        FSM::friendIDs.push_back(std::stoi(ID));
        pt++;

        std::string name = "";
        while (*pt != ','){
            name += *pt;
            pt++; 
        }
        FSM::friends.push_back(name);
        pt++;
    }
    pt++;

    while (*pt != ';') {
        if(*pt == 'N'){
            pt += 16;
            break;
        }

        std::string ID = "";
        while (*pt != ','){
            ID += *pt;
            pt++; 
        }
        FSM::incoming_friendIDs.push_back(std::stoi(ID));
        pt++;

        std::string name = "";
        while (*pt != ','){
            name += *pt;
            pt++; 
        }
        FSM::incoming_friends.push_back(name);
        pt++;
    }
    pt++;

    while (*pt != ';') {
        if(*pt == 'N'){
            pt += 16;
            break;
        }

        std::string ID = "";
        while (*pt != ','){
            ID += *pt;
            pt++; 
        }
        FSM::outgoing_friendIDs.push_back(std::stoi(ID));
        pt++;

        std::string name = "";
        while (*pt != ','){
            name += *pt;
            pt++; 
        }
        FSM::outgoing_friends.push_back(name);
        pt++;
    }
}

void FSM::parseInvites(char* response) {
    FSM::inviterNames.erase_all();
    FSM::inviterIDs.erase_all();
    FSM::inviteGameCode.erase_all();

    char* pt = response + 9;

    while (*pt != '\0') {
        std::string ID = "";
        while (*pt != ','){
            ID += *pt;
            pt++; 
        }
        FSM::inviterIDs.push_back(std::stoi(ID));
        pt++;

        std::string name = "";
        while (*pt != ','){
            name += *pt;
            pt++; 
        }
        FSM::inviterNames.push_back(name);
        pt++;

        std::string gameCode = "";
        while (*pt != ';'){
            gameCode += *pt;
            pt++; 
        }
        FSM::inviteGameCode.push_back(std::stoi(gameCode));
        pt++;
    }
}

void PORT8_IRQHandler() {
    menu.setButtonInput(P8->IFG);

    P8->IFG &= ~(BIT3 | BIT4 | BIT5 | BIT6 | BIT7);
    P8->IE &= ~(BIT3 | BIT4 | BIT5 | BIT6 | BIT7);
}
