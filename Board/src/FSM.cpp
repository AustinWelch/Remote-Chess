#include "FSM.h"
#include "LCD_CharacterDisplay.h"
#include "ButtonInterface.h"
#include "G8RTOS_Scheduler.h"
#include <cstdint>

using namespace RemoteChess;

extern Board g_board;
ButtonInterface g_buttons;

static char serverResponse[512];

void FSM::FSMController() {
    lcd.Init();

    while (true) {
        if (didChangeState) {
            lcd.Clear();
            didChangeState = false;
        }

        switch (curState) {
            case FSM::State::INITIAL_CONNECTION:
                FSM::InitialConnection();
                break;
            // case FSM::State::INITIAL_WIFI_CHANGE:
            //     FSM::InitialWIFIChange();
            //     break;
            case FSM::State::DOWNLOAD_CURRENT_GAME:
                FSM::DownloadCurrentGame();
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
            // case FSM::State::SETTINGS_BOARDPREFERENCES:
            //     FSM::SettingsBoardPreferences();
            //     break;
            // case FSM::State::SETTINGS_WIFI:
            //     FSM::SettingsWifi();
            //     break;
            case FSM::State::JOIN_CREATE:
                FSM::JoinCreate();
                break;
            case FSM::State::CREATE:
                FSM::Create();
                break;
            case FSM::State::CPU_GAME:
                FSM::CPUGame();
                break;
            case FSM::State::JOIN:
                FSM::Join();
                break;
            case FSM::State::INCOMING_INVITE:
                FSM::IncomingInvite();
                break;
            case FSM::State::NOGAME:
                FSM::NoGame();
                break;
            case FSM::State::INGAME:
                FSM::InGame();
                break;
            case FSM::State::RESIGN:
                FSM::Resign();
                break;
            default:
                G8RTOS_SleepThread(1000);
                break;
        }

        if (curState != nextState) {
            curState = nextState;
            didChangeState = true;
        }
    }
}

//TODO: Set AP credentials in sl_common.h to those stored in flash
void FSM::InitialConnection() {
    lcd.WriteLine("Connecting to server...", 0);

    int attempts = 0;
    while (attempts < 3) {
        if (chessServer_init(KEEP_CONNECTION))
            break;

        attempts++;
        G8RTOS_SleepThread(LCD_DISPLAY_TIME);
    }

    if (attempts == 3) { 
        // lcd.WriteLineCentered("Failed to connect to server", 0);
        // uint8_t resp = menu.DisplayMenuLeft(lcd, {"Retry", "Update WIFI Credentials", "", ""}, 1, 2);
        // if (resp == 1) 
        //     nextState = FSM::State::INITIAL_CONNECTION;
        // else
        //     nextState = FSM::State::INITIAL_WIFI_CHANGE;

        // return;
    }

    if (chessServer_getCurrentGame() == SUCCESS) {
        isGameRunning = true;
        nextState = State::DOWNLOAD_CURRENT_GAME;
    } else {
        nextState = State::NOGAME;
    }
}

// void FSM::InitialWIFIChange() {
//     lcd.WriteLine("Enter AP Name: ", 0);
//     //char newAPName[100] = BoardKeyBoardFunction(); //Enter letters from sensors, print letters to LCD, middle button to submit

//     lcd.WriteLine("Enter AP Pass: ", 1);
    
//     //char newAPPass[100] = BoardKeyBoardFunction();

//     //Save new credentials to flash

//     //memcpy(SSID_NAME, newAPName, strlen(newAPName)); 
//     //memcpy(PASSKEY, newAPPass, strlen(newAPPass));

//     nextState = FSM::State::INITIAL_CONNECTION;
// }

void FSM::DownloadCurrentGame() {
    lcd.WriteLine("Downloading game...", 0);
	g_board.UpdateFromServer();
    
    nextState = State::INGAME;
}

void FSM::Main_Menu() {
    char titleTemp[] = "Welcome, %s";
    char title[20];
    char response[20];
    chessServer_getName(response);

    sprintf(title, titleTemp, response + 6);

    lcd.WriteLineCentered(title, 0);
    
    uint8_t buttonResp;

    if (isGameRunning) {
        buttonResp = menu.DisplayMenuLeft(lcd, {"Friends", "Settings", "Back", ""}, 1, 3);

        if (buttonResp == 0) {
            nextState = FSM::State::FRIENDS;
        } else if (buttonResp == 1) {
            nextState = FSM::State::SETTINGS;
        } else {
            nextState = FSM::State::INGAME;
        }
    } else {
        buttonResp = menu.DisplayMenuLeftRight(lcd, {"Play", "Friends", "Settings", "Back", "", "", "", ""}, 1, 4);
        
        if (buttonResp == 0) {
            nextState = FSM::State::JOIN_CREATE;
        } else if (buttonResp == 1) {
            nextState = FSM::State::FRIENDS;
        } else if (buttonResp == 2) {
            nextState = FSM::State::SETTINGS;
        } else if (buttonResp == 3) {
            nextState = FSM::State::NOGAME;
        } 
    }
}

void FSM::Friends() {
    int retVal = chessServer_getFriends(serverResponse);
    parseFriends(serverResponse);
    
    if (retVal == INVALID_RESPONSE || retVal == REQUEST_FAILED) {
        lcd.WriteMessageWrapped(serverResponse);
        G8RTOS_SleepThread(LCD_DISPLAY_TIME);
        return;
    }

    lcd.WriteLine("Friends" , 0);   
    lcd.WriteLineRight("ID:     ", 0);
    lcd.WriteLineRight(boardIDStr, 0);

    if (retVal == NO_FRIENDS) {
        lcd.WriteLineCentered("No friends added", 1);

        uint8_t buttonResponse = menu.DisplayMenuLeftRight(lcd, {"Add", "Back", "", "", "", "", "", ""}, 3, 2);

        if (buttonResponse == 0)
            nextState = FSM::State::FRIENDS_ADD;
        else 
            nextState = FSM::State::MAIN_MENU;

        return;
    } else {
        RemoteChess::flat_vector<const char*, 10> friendNames;

        for (const Friend& fr : friends) {
            friendNames.push_back(fr.name);
        }

        int8_t selection = menu.DisplayScrollingMenu(lcd, friendNames, friends.size(), "Add");

        if (selection == -2) {
            nextState = FSM::State::FRIENDS_ADD;
        } else if (selection == -1) {
            nextState = FSM::State::MAIN_MENU;
        } else {
            currentFriend = &friends[selection];
            
            nextState = FSM::State::FRIENDS_SELECT;
        }
    }
}

void FSM::FriendsSelect() {
    lcd.WriteLineCentered(currentFriend->name, 0);
    
    uint8_t buttonResponse = menu.DisplayMenuLeft(lcd, {"Invite to Game", "Remove Friend", "Back", ""}, 1, 3);
  
    if (buttonResponse == 0) {
        nextState = FSM::State::FRIENDS_SELECT_INVITE;
    } else if (buttonResponse == 1) {
        nextState = FSM::State::FRIENDS_SELECT_REMOVE;
    } else {
        currentFriend = nullptr;
        nextState = FSM::State::FRIENDS;
    } 
}

Semaphore waitingForFriendSem = { 0 };
AwaitingMove friendStatus;

void WaitForFriend(void) {
    while (true) {
        friendStatus = chessServer_awaitTurn();

        if (friendStatus.status == SUCCESS) {
            G8RTOS_ReleaseSemaphore(&waitingForFriendSem); // Locked by Create thread waiting for our response
            G8RTOS_KillSelf();
        }
        G8RTOS_SleepThread(SERVER_PING_DELAY);
    }
}

void FSM::FriendsSelectInvite() {

    int8_t retVal = chessServer_sendInvite(serverResponse, currentFriend->id);

    if (retVal == SUCCESS) {
        lcd.Clear();

        NewThreadStatus waitForFriendThr = G8RTOS_AddThread(WaitForFriend, 10, "WaitFriend");

        if (waitForFriendThr.status == G8RTOS_SUCCESS) {
            char name[20];
            lcd.WriteLineCentered("Waiting for", 0);
            sprintf(name, "%s to join", currentFriend->name);
            lcd.WriteLineCentered(name, 1);

            lcd.WriteLineCentered("(*) Cancel", 3);

            while (true) {
                if (G8RTOS_IsSemaphoreAvailable(&waitingForFriendSem)) {
                    G8RTOS_AcquireSemaphore(&waitingForFriendSem); // Semaphore released, friend joined game
                
                    nextState = State::DOWNLOAD_CURRENT_GAME;
                    break;
                } else {
                    // Invite not yet accepted
                    ButtonState btnState = g_buttons.GetCurrentButtonStatePoll();

                    if (btnState.center) {
                        chessServer_deleteGame();
                        G8RTOS_KillThread(waitForFriendThr.id);

                        nextState = FSM::State::FRIENDS_SELECT;
                        break;
                    } 
                }
            }
        }
    } else {
        lcd.WriteMessageWrapped(serverResponse);
        G8RTOS_SleepThread(LCD_DISPLAY_TIME);
        nextState = FSM::State::FRIENDS_SELECT;
    }
}

void FSM::FriendsSelectRemove() {
   lcd.WriteLineCentered("Are you sure you", 0);
   lcd.WriteLineCentered("want to to remove", 1);
   lcd.WriteLineCentered(currentFriend->name, 2);

   uint8_t buttonResponse = menu.DisplayMenuLeftRight(lcd, {"Yes", "No", "", "", "", "", "", ""}, 3, 2);
   if (buttonResponse == 0) {
       int8_t retVal = chessServer_removeFriend(serverResponse, currentFriend->id);
       lcd.WriteMessageWrapped(serverResponse);
       G8RTOS_SleepThread(LCD_DISPLAY_TIME);
   }

   nextState = FSM::State::FRIENDS;
}

void FSM::FriendsAdd() {
    while (true) {
        lcd.Clear();

        uint8_t buttonResponse = menu.DisplayMenuLeft(lcd, { "Send Request", "Incoming Requests", "Back" }, 0, 3);

        if (buttonResponse == 0) {            
            char friendID[7];
            memset(friendID, '\0', 7);

            int8_t retVal = menu.KeyboardInputNumber(lcd, "Friend ID: ", friendID);

            if (retVal == 1) {
                int ID;
                sscanf(friendID, "%d", &ID);
                retVal = chessServer_addFriend(serverResponse, ID);

                lcd.Clear();
                lcd.WriteMessageWrapped(serverResponse);
                G8RTOS_SleepThread(LCD_DISPLAY_TIME);
                if (retVal == INVALID_RESPONSE || retVal == REQUEST_FAILED)
                    nextState = FSM::State::FRIENDS_ADD;
                    return;
            }
        }

        else if (buttonResponse == 1) {
            while (true) {
                lcd.Clear();
                lcd.WriteLineCentered("Incoming Requests", 0);

                RemoteChess::flat_vector<const char*, 10> incomingFriendsNames;

                for (const Friend& fr : incomingFriends) {
                    incomingFriendsNames.push_back(fr.name);
                }
                        
                int8_t selection = menu.DisplayScrollingMenu(lcd, incomingFriendsNames, incomingFriendsNames.size(), "");

                if (selection >= 0) {
                    lcd.Clear();
                    lcd.WriteLineCentered(incomingFriends[selection].name, 0);
                    
                    buttonResponse = menu.DisplayMenuLeft(lcd, {"Accept", "Decline", "Back", ""}, 1, 3);

                    if (buttonResponse == 0) {
                        lcd.Clear();
                        chessServer_acceptFriend(serverResponse, incomingFriends[selection].id);

                        incomingFriends.erase(incomingFriends[selection]);

                        lcd.WriteMessageWrapped(serverResponse);
                        G8RTOS_SleepThread(LCD_DISPLAY_TIME);
                    }
                    else if (buttonResponse == 1) {
                        lcd.Clear();
                        chessServer_declineFriend(serverResponse, incomingFriends[selection].id);

                        incomingFriends.erase(incomingFriends[selection]);

                        lcd.WriteMessageWrapped(serverResponse);
                        G8RTOS_SleepThread(LCD_DISPLAY_TIME);
                    }
                }
                else
                    break;
            }
        }

//        else if (buttonResponse == 2) {
//            while (true) {
//                lcd.Clear();
//                lcd.WriteLineCentered("Outgoing Requests", 0);
//
//                RemoteChess::flat_vector<const char*, 10> outgoingFriendsNames;
//
//                for (const Friend& fr : friends) {
//                    outgoingFriendsNames.push_back(fr.name);
//                }
//
//                int8_t selection = menu.DisplayScrollingMenu(lcd, outgoingFriendsNames, outgoingFriends.size(), "");
//
//                if (selection > 0) {
//                    lcd.Clear();
//                    lcd.WriteLineCentered(outgoingFriends[selection].name, 0);
//
//                    buttonResponse = menu.DisplayMenuLeft(lcd, {"Cancel", "Back", "", ""}, 1, 2);
//
//                    if (buttonResponse == 0) {
//                        lcd.Clear();
//                        chessServer_cancelFriend(serverResponse, outgoingFriends[selection].id);
//
//                        outgoingFriends.erase(outgoingFriends[selection]);
//
//                        lcd.WriteMessageWrapped(serverResponse);
//                        G8RTOS_SleepThread(LCD_DISPLAY_TIME);
//                    }
//                    else {
//                        continue;
//                    }
//                }
//                else
//                    break;
//            }
//        }

        else
            break;
    }
    
    nextState = FSM::State::FRIENDS;
}

void FSM::Settings() {
    lcd.WriteLineCentered("Settings", 0);
    uint8_t buttonResponse = menu.DisplayMenuLeftRight(lcd, {"Board Pref", "WIFI", "Name", "Back", "", "", "", ""}, 1, 4);
   
    if (buttonResponse == 0) {
        //nextState = FSM::State::SETTINGS_BOARDPREFERENCES;
    } else if (buttonResponse == 1) {
        //nextState = FSM::State::SETTINGS_WIFI;
    } else if (buttonResponse == 2) {
        //nextState = FSM::State::SETTINGS_NAMECHANGE;
    } else {
        nextState = FSM::State::MAIN_MENU;
    }
}

// void FSM::SettingsNameChange() {
//     lcd.WriteLineCentered("Enter new name: ", 1);
    
//     char newName[100];
//     uint8_t retVal = 0;//BoardKeyBoardFunction(); //Enter letters from sensors, print letters to LCD, middle button to submit

//     if (retVal == 1) {
//         char response[1024];
//         chessServer_setName(response, newName);
//         lcd.WriteMessageWrapped(response);
//         DelayMs(LCD_DISPLAY_TIME);
//     }

//     nextState = FSM::State::SETTINGS;
// }

// void FSM::SettingsBoardPreferences() {
//     //TODO: initialize variables in ChessBoard for lights and sound, load those settings form flash and just change runtime and write here
//     //Write current setting next to menu selection 

//     while(true){
//         lcd.WriteLineCentered("Board Preferences", 0);
        
//         uint8_t buttonResponse = menu.DisplayMenuLeft(lcd, {"Assist Lights:", "Sound:", "Back", ""}, 1, 3);

//         if (buttonResponse == 1) {
//             //usingLights = usingLights;
//             //save settings to flash or db
//         } else if (buttonResponse == 2) {
//             //usingSound = !usingSound;
//             //save settings to flash or db
//         } else {
//             break;
//         }
//     }

//     nextState = FSM::State::SETTINGS;
// }

// void FSM::SettingsWifi() {
//     nextState = FSM::State::SETTINGS;

//     lcd.WriteLine("Enter AP Name:", 0);
    
//     char newAPName[100]; 
//     uint8_t retVal = 0;//BoardKeyBoardFunction(newAPName); //Enter letters from sensors, print letters to LCD, middle button to submit

//     if (retVal == 0) {
//         return;
//     }
    
//     lcd.WriteLine("Enter AP Pass:", 2);
    
//     char newAPPass[100]; 
//     retVal = 0;//BoardKeyBoardFunction(newAPPass);

//     if (retVal == 0) {
//         return;
//     }

//     //Save new credentials to flash

//     //memcpy(SSID_NAME, newAPName, strlen(newAPName)); 
//     //memcpy(PASSKEY, newAPPass, strlen(newAPPass));
// }

void FSM::JoinCreate() {
    uint8_t buttonResponse = menu.DisplayMenuLeft(lcd, { "Join with code", "Create Game", "Versus CPU", "Back" }, 0, 4);

    if (buttonResponse == 0) {
        nextState = FSM::State::JOIN;
    } else if (buttonResponse == 1) {
        nextState = FSM::State::CREATE;
    } else if (buttonResponse == 2) {
        nextState = FSM::State::CPU_GAME;
    } else {
        nextState = FSM::State::MAIN_MENU;        
    }
}

Semaphore waitingForOpponentSem = { 0 };
AwaitingMove opponentStatus;

void WaitForOpponent(void) {
    while (true) {
        opponentStatus = chessServer_awaitTurn();

        if (opponentStatus.status == SUCCESS) {
            G8RTOS_ReleaseSemaphore(&waitingForOpponentSem); // Locked by Create thread waiting for our response
            G8RTOS_KillSelf();
        }
        G8RTOS_SleepThread(SERVER_PING_DELAY);
    }
}

void FSM::Create() {
    int8_t retVal = chessServer_newGame(serverResponse);

    if (retVal == SUCCESS) {

        NewThreadStatus waitForOpponentThr = G8RTOS_AddThread(WaitForOpponent, 10, "WaitOpponent");

        if (waitForOpponentThr.status == G8RTOS_SUCCESS) {
            while (true) {
                if (G8RTOS_IsSemaphoreAvailable(&waitingForOpponentSem)) {
                    G8RTOS_AcquireSemaphore(&waitingForOpponentSem); // Semaphore released, opponent joined game
                
                    nextState = State::DOWNLOAD_CURRENT_GAME;
                    break;
                } else {
                    // No invite received
                    lcd.WriteLine("Game code: ", 0);
                    lcd.WriteLineRight(chessServer_getGameCode(), 0);
                    lcd.WriteLineCentered("Waiting for", 1);
                    lcd.WriteLineCentered("opponent to join", 2);
                    lcd.WriteLineCentered("(*) Cancel", 3);

                    ButtonState btnState = g_buttons.GetCurrentButtonStatePoll();

                    if (btnState.center) {
                        chessServer_deleteGame();
                        G8RTOS_KillThread(waitForOpponentThr.id);

                        nextState = State::JOIN_CREATE;
                        break;
                    } 
                }
            }
        }
    } else {
        lcd.WriteMessageWrapped(serverResponse);
        G8RTOS_SleepThread(LCD_DISPLAY_TIME);
        nextState = State::JOIN_CREATE;
    }
}

void FSM::CPUGame() {
    int8_t retVal = chessServer_newGameCPU(serverResponse);

    if (retVal == SUCCESS) {
        isCPUgame = true;
        nextState = State::DOWNLOAD_CURRENT_GAME;
    } else {
        lcd.WriteMessageWrapped(serverResponse);
        G8RTOS_SleepThread(LCD_DISPLAY_TIME);
        nextState = State::JOIN_CREATE;
    }
}

void FSM::Join() {
    char gameCode[10];
    memset(gameCode, '\0', 10);

    uint8_t retVal = menu.KeyboardInputNumber(lcd, "Game Code: ", gameCode); //Enter characters from sensors, print characters to LCD, middle button to submit
    
    if (retVal == 1) {
        chessServer_setGameCode(gameCode);
        retVal = chessServer_joinGame(serverResponse);

        lcd.Clear()
        lcd.WriteMessageWrapped(serverResponse);
        G8RTOS_SleepThread(LCD_DISPLAY_TIME);

        if(retVal == SUCCESS) {
            nextState = FSM::State::DOWNLOAD_CURRENT_GAME;
            return;
        } 
    }     

    nextState = FSM::State::JOIN_CREATE;
}

Semaphore inviteSem = { 0 };
ServerGameInvite incomingInvite;

void FSM::IncomingInvite() {
    // Invite received!
    char playQuestion[21];
    sprintf(playQuestion, "Play with %s?", incomingInvite.playerName);

    lcd.WriteLineCentered(playQuestion, 0);

    uint8_t buttonResponse = menu.DisplayMenuLeftRight(lcd, { "Yes", "No", "", "", "", "", "", "" }, 3, 2);
            
    if (buttonResponse == 0) {
        uint8_t retVal = chessServer_acceptInvite(incomingInvite.playerId);

        if (retVal == SUCCESS) {
            chessServer_setGameCode(incomingInvite.gamecode);

            nextState = FSM::State::DOWNLOAD_CURRENT_GAME;
            return;
        } else {
            
        }
    } else {
        uint8_t retVal = chessServer_declineInvite(incomingInvite.playerId);

        if (retVal == SUCCESS) {
            nextState = FSM::State::NOGAME;
        }
    }
}

void WaitForInvite(void) {
    while (true) {
        incomingInvite = chessServer_getLastInvite();

        if (incomingInvite.status == SUCCESS) {
            G8RTOS_ReleaseSemaphore(&inviteSem); // Locked by NoGame thread waiting for our response
            G8RTOS_KillSelf();
        }

        G8RTOS_SleepThread(SERVER_PING_DELAY);
    }

}

void FSM::NoGame() {
    isGameRunning = false;
    isCPUgame = false;

    NewThreadStatus waitForInviteThr = G8RTOS_AddThread(WaitForInvite, 10, "WaitInvite");

    if (waitForInviteThr.status == G8RTOS_SUCCESS) {
        while (true) {
            if (G8RTOS_IsSemaphoreAvailable(&inviteSem)) {
                G8RTOS_AcquireSemaphore(&inviteSem); // Not released, released by WaitForInvite, WaitForInvite is dead
            
                nextState = State::INCOMING_INVITE;
                break;
            } else {
                // No invite received
                lcd.WriteLineCentered("Not in game", 0);
                lcd.WriteLineCentered("Waiting for invite", 1);
                lcd.WriteLineCentered("(^) Main Menu", 3);

                ButtonState btnState = g_buttons.GetCurrentButtonStatePoll();

                if (btnState.up) {
                    G8RTOS_KillThread(waitForInviteThr.id);

                    nextState = State::MAIN_MENU;
                    break;
                }
            }
        }
    }
}

void FSM::InGame() {
    auto lastMove = g_board.GetLastMove();

    if (lastMove.HasValue()) {
        char buffer[21];

        sprintf(buffer, "Last Move: %s%c", g_board.GetLastMove()->GetAlgabreic().data(), g_board.IsInCheck() ? '+' : '\0');
        lcd.WriteFullLineCentered(buffer, 0);
    }

    ButtonState btnState = g_buttons.GetCurrentButtonStatePoll();

    if (g_board.GetCurrentState() == Board::BoardState::AWAITING_LOCAL_MOVE) {
        lcd.WriteFullLineCentered("(^) Main Menu", 2);
        lcd.WriteFullLineCentered("(v) Resign", 3);

        if (g_board.IsPotentialLocalMoveValid()) {
            lcd.WriteFullLineCentered("(*) Submit Move", 1);
        } else {
            lcd.WriteFullLineCentered("Make your move", 1);
        }
        
        
        if (btnState.center) {
            RemoteChess::optional<Move> localMove = g_board.SubmitCurrentLocalMove();

            if (localMove.HasValue()) {
                chessServer_makeMove(localMove->GetAlgabreic().data());
                lcd.Clear();
            }
        } else if (btnState.up) {
            nextState = State::MAIN_MENU;
            return;
        } else if (btnState.down) {
            nextState = State::RESIGN;
            return;
        }

        G8RTOS_SleepThread(10);
    } else if (g_board.GetCurrentState() == Board::BoardState::AWAITING_REMOTE_MOVE_NOTICE) {
        lcd.WriteFullLineCentered("Opponent's move...", 1);
        lcd.WriteFullLineCentered("(^) Main Menu ", 3);

        AwaitingMove awaitingMove = chessServer_awaitTurn();

        if (awaitingMove.status == SUCCESS) {
            Move remoteMove(awaitingMove.algabreic);

            g_board.ReceiveRemoteMove(remoteMove);

            if (awaitingMove.inCheck) {
                g_board.SetCheck(Cell(awaitingMove.algabreicKingPosCheck));
            }

            g_board.UpdateLegalMoves();

            lcd.Clear();
            return; 
        } else if (awaitingMove.status == SERVER_WON_GAME) {
            g_board.WinGame(Cell(awaitingMove.algabreicKingPosWinner), Cell(awaitingMove.algabreicKingPosCheck));
            lcd.Clear();

            return;
        } else if (awaitingMove.status == SERVER_LOST_GAME) {
            Move remoteMove(awaitingMove.algabreic);

            g_board.ReceiveRemoteMove(remoteMove);
            g_board.SetUpcomingCheckmate(Cell(awaitingMove.algabreicKingPosWinner), Cell(awaitingMove.algabreicKingPosCheck));

            lcd.Clear();
            return;
        } else if (awaitingMove.status == SERVER_OPP_RESIGNED) {
            g_board.WinGame();

            lcd.Clear();
            lcd.WriteFullLineCentered("You won", 0);
            lcd.WriteFullLineCentered("by resignation!", 1);
            lcd.WriteFullLineCentered("(*) Continue", 3);

            while (true) {
                if (g_buttons.GetCurrentButtonState().center) {
                    g_board.GoToIdle();

                    chessServer_leaveGame();

                    nextState = State::NOGAME;
                    isGameRunning = false;
                    break;
                }
            }
        }

        if (btnState.up) {
            nextState = State::MAIN_MENU;
            return;
        }

        G8RTOS_SleepThread(isCPUgame ? 500 : 1000);
    } else if (g_board.GetCurrentState() == Board::BoardState::AWAITING_REMOTE_MOVE_FOLLOWTHROUGH) {
        lcd.WriteLineCentered("Perform opponent's", 1);
        lcd.WriteLineCentered("move.", 2);
    } else if (g_board.GetCurrentState() == Board::BoardState::WON_GAME) {
        lcd.WriteFullLineCentered("You won!", 1);
        lcd.WriteFullLineCentered("(*) Continue", 2);

        while (true) {
            if (g_buttons.GetCurrentButtonState().center) {
                g_board.GoToIdle();

                chessServer_leaveGame();

                nextState = State::NOGAME;
                isGameRunning = false;
                break;
            }
        }
    } else if (g_board.GetCurrentState() == Board::BoardState::LOST_GAME) {
        lcd.WriteFullLineCentered("You lost!", 1);
        lcd.WriteFullLineCentered("(*) Continue", 2);

        while (true) {
            if (g_buttons.GetCurrentButtonState().center) {
                g_board.GoToIdle();

                chessServer_leaveGame();

                nextState = State::NOGAME;
                isGameRunning = false;
                break;
            }
        }
    }
}

void FSM::Resign() {
    lcd.WriteLineCentered("Resign from Game?", 0);
    
    uint8_t buttonResponse = menu.DisplayMenuLeft(lcd, { "Yes", "No", "", "" }, 2, 2);
   
    if (buttonResponse == 0) {
        lcd.Clear();
        lcd.WriteLineCentered("Resigning from game", 0);
        nextState = FSM::State::MAIN_MENU;
        while (true) {
            int8_t retVal = chessServer_resign();

            if (retVal == SUCCESS) {
                lcd.Clear();
                lcd.WriteFullLineCentered("You resigned!", 1);
                lcd.WriteFullLineCentered("(*) Continue", 2);

                g_board.LoseGame();

                while (true) {
                    if (g_buttons.GetCurrentButtonState().center) {
                        g_board.GoToIdle();

                        chessServer_leaveGame();

                        nextState = State::NOGAME;
                        isGameRunning = false;
                        return;
                    }
                }
            } else {
                continue;
            }
        }
    } else {
        nextState = State::INGAME;
    }
}


// HELPER FUNCTIONS

void FSM::parseFriends(char* response) {
    friends.clear();
    incomingFriends.clear();
    outgoingFriends.clear();

    char* pt = response + 9;
    int i;
    int temp;

    while (*pt != ';') {
        if(*pt == 'N'){
            pt += 10;
            break;
        }

        Friend friendToAdd;

        i = 0;
        char ID[7];
        while (*pt != ','){
            ID[i] = *pt;
            pt++; i++;
        }
        ID[i] = '\0';

        sscanf(ID, "%d", &temp);
        friendToAdd.id = temp;
        pt += 2;

        i = 0;
        while (*pt != ','){
            friendToAdd.name[i] = *pt;
            pt++; i++;
        }
        friendToAdd.name[i] = '\0';

        pt += 2;

        friends.push_back(friendToAdd);
    }
    pt++;

    while (*pt != ';') {
        if(*pt == 'N'){
            pt = strstr(pt, ";");
            break;
        }

        Friend incomingFriend;

        i = 0;
        char ID[7];
        while (*pt != ','){
            ID[i] = *pt;
            pt++; i++;
        }
        ID[i] = '\0';

        sscanf(ID, "%d", &temp);
        incomingFriend.id = temp;
        pt += 2;

        i = 0;
        while (*pt != ','){
            incomingFriend.name[i] = *pt;
            pt++; i++;
        }
        incomingFriend.name[i] = '\0';

        pt += 2;

        incomingFriends.push_back(incomingFriend);
    }
    pt++;

   while (*pt != ';') {
        if(*pt == 'N'){
            break;
        }

        Friend outgoingFriend;

        i = 0;
        char ID[7];
        while (*pt != ','){
            ID[i] = *pt;
            pt++; i++;
        }
        ID[i] = '\0';

        sscanf(ID, "%d", &temp);
        outgoingFriend.id = temp;
        pt += 2;

        i = 0;
        while (*pt != ','){
            outgoingFriend.name[i] = *pt;
            pt++; i++;
        }
        outgoingFriend.name[i] = '\0';

        pt += 2;

        outgoingFriends.push_back(outgoingFriend);
    }
    pt++;
}
