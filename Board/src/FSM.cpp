/*
    TODO:
    Standardize LCD API

    Make an input API, using the LCD cursor feature
        -Bind cursor to displayed list
        -Get selection value based on cursor position
        -Try to get left and right selections

    Scroll thing for friends and long lists

    Board keyboard function

    COMPLETE FSM FUNCTIONALITY
        -d e b u g
        -Put FSM code in switch statement?

    Get piece placement w/ Lights

    Get Flash memory to work (if possible)

    Single Player vs Online AI
        
*/

#include "FSM.h"
#include "Board.h"

using namespace RemoteChess;

void FSM::FSMController() {
    while (true) {
        //LCD_Clear();

        switch (FSM::State::curState) {
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
        //LCD_TextOut("Failed to Connect to Server.")
        //LCD_NextLine()
        //LCD_TextOut("-Retry -Update WIFI Credentials")

        //Button_WaitForResp()

        int resp = 0; //Button_Response();
        if (resp == 1) //Retry
            nextState = FSM::State::INITIAL_CONNECTION;
        else
            nextState = FSM::State::INITIAL_WIFI_CHANGE;

        return;
    }

    nextState = FSM::State::MAIN_MENU;
}

void FSM::InitialWIFIChange() {
    //LCD_TextOut("Enter AP Name:")
    //LCD_NextLine()
    //char newAPName[100] = BoardKeyBoardFunction(); //Enter letters from sensors, print letters to LCD, middle button to submit

    //LCD_NextLine()
    //LCD_TextOut("Enter AP Name:")
    //LCD_NextLine()
    //char newAPPass[100] = BoardKeyBoardFunction();

    //Save new credentials to flash

    //memcpy(SSID_NAME, newAPName, strlen(newAPName)); 
    //memcpy(PASSKEY, newAPPass, strlen(newAPPass));

    nextState = FSM::State::INITIAL_CONNECTION;
}

void FSM::Main_Menu() {
    //LCD_TextOut("Remote Chess")
    //LCD_NextLine()
    //LCD_TextOut("Game")
    //LCD_NextLine()
    //LCD_TextOut("Friends")
    //LCD_NextLine()
    //LCD_TextOut("Settings")

    //Button_WaitForResp()

    int resp = 0; //Button_Response();
    if (resp == 1)
        nextState = FSM::State::FIND_GAME;

    if (resp == 2)
        nextState = FSM::State::FRIENDS;

    if (resp == 3)
        nextState = FSM::State::SETTINGS;
}

//TODO: Handle invites locally and on server
void FSM::Friends() {
    char response[1024];
    int retVal = chessServer_getFriends(response);
    parseFriends(response);
    
    if (retVal == INVALID_RESPONSE || retVal == REQUEST_FAILED) {
        //LCD_TextOut(response)
        //wait 3 seconds
        nextState = FSM::State::FRIENDS; 
        return;
    }

    //LCD_TextOut(userName + " ID: " + BOARD_ID)
    //LCD_NextLine()

    if (retVal == NO_FRIENDS) {
        //LCD_TextOut(response)
        //LCD_NextLine()
        //LCD_TextOut("Add Friend       Friends")

        //Button_WaitForResp()

        //If adding a friend
        //nextState = FSM::State::FRIENDS_ADD;
        
        //else, selecting back
        nextState = FSM::State::MAIN_MENU;
    }

    //Display friends and create scrolling mechanism for more than 3 friends

    //If adding a friend
        //nextState = FSM::State::FRIENDS_ADD;

    //If selecting a friend
        //currentlyFriendID = selectedFriendID
        //currentlyFriendName = selectedFriendName
        //nextState = FSM::State::FRIENDS_SELECT;

    //If selecting back
        nextState = FSM::State::MAIN_MENU;
}

void FSM::FriendsAdd() {
    while (true) {
        //LCD_Clear()
        //LCD_TextOut("Send Request")
        //LCD_NextLine()
        //LCD_TextOut("Incoming Requests")
        //LCD_NextLine()
        //LCD_TextOut("Outgoing Requests")
        //LCD_NextLine()
        //LCD_TextOut("Back")

        //if (buttonResp == 0)
            //LCD_Clear()
            //LCD_TextOut("Enter Friend's ID")
            //LCD_NextLine()
            //LCD_TextOut("ID: ")
            //LCD_NextLine()
            //char friendID[100] = BoardKeyBoardFunction();

            //if submit
                char response[1024];
                //int resp = chessServer_addFriend(response, std::stoi(convertToString(friendID)));
                //LCD_Display(convertToString(response))
                //Delay 3 Seconds
                if (resp == INVALID_RESPONSE || resp == REQUEST_FAILED)
                    nextState = FSM::State::FRIENDS_ADD;
                    return;

            //if back
                continue;

        //if (buttonResp == 1)
            while (true) {
                //LCD_Clear()
                //LCD_TextOut("Incoming Friend Requests")
                //LCD_NextLine()
                //Scrolling function with incoming friend reqs

                //if select a friend
                //LCD_Clear()
                //LCD_TextOut(incoming_friends[selectionID])
                //LCD_NextLine()
                //LCD_TextOut("Accept")
                //LCD_NextLine()
                //LCD_TextOut("Decline")
                //LCD_NextLine()
                //LCD_TextOut("Back")

                //if (buttonResp == 1) 
                    //chessServer_acceptFriend(serverResponse, selectionID)
                    //LCD_TextOut(serverResponse)
                    //Wait 3 seconds

                //if (buttonResp == 2) 
                    //chessServer_declineFriend(serverResponse, selectionID)
                    //LCD_TextOut(serverResponse)
                    //Wait 3 seconds

                //if (buttonResp == 3)
                    break;

                continue;
            }

        //if (buttonResp == 2)
            while (true) {
                //LCD_Clear()
                //LCD_TextOut("Outgoing Friend Requests")
                //LCD_NextLine()
                //Scrolling function with outgoing friend reqs

                //if select a friend
                //LCD_Clear()
                //LCD_TextOut(outgoing_friends[selectionID])
                //LCD_NextLine()
                //LCD_TextOut("Cancel")
                //LCD_NextLine()
                //LCD_TextOut("Back")
                //LCD_NextLine()

                //if (buttonResp == 1) 
                    //chessServer_cancelFriend(serverResponse, selectionID)
                    //LCD_TextOut(serverResponse)
                    //Wait 3 seconds

                //if (buttonResp == 2) 
                    break;

                continue;
            }

        //if (buttonResp == 3)
            break;
    }
    
    //go back
    nextState = FSM::State::FRIENDS;
}

void FSM::FriendsSelect() {
    //LCD_TextOut(currentFriendName)
    //LCD_NextLine()
    //LCD_TextOut("Invite to Game")
    //LCD_NextLine()
    //LCD_TextOut("Remove Friend")
    //LCD_NextLine()
    //LCD_TextOut("Back")

    //Button_WaitForResp()

    //if (resp == 1)
        nextState = FSM::State::FRIENDS_SELECT_INVITE;

    //if (resp == 2)
        nextState = FSM::State::FRIENDS_SELECT_REMOVE;

    //if (resp == 3)
        nextState = FSM::State::FRIENDS;
}

void FSM::FriendsSelectInvite() {
    char response[1024];
    //Send invite
    //LCD_TextOut(response)
    //Wait 3 seconds
    //if (retVal == SUCCESS) 
        //LCD_TextOut("Waiting for 'currentFriendName' to join")
        //LCD_NextLine()
        //LCD_TextOut("Press any button to cancel")

        //once p2 joins
        //joiningAsPlayer = PlayerColor::WHITE;
        //nextState = FSM::State::INGAME;
            //return

        //if button is pressed
            //Cancel invite
            //nextState = FSM::State::FRIENDS_SELECT;
    
    //else
        nextState = FSM::State::FRIENDS_SELECT;
}

void FSM::FriendsSelectRemove() {
    //LCD_TextOut("Are you sure you want to remove 'currentFriendName")
    //LCD_NextLine()
    //LCD_TextOut("Yes  No")

    //Button_WaitForResp()
    //if (resp == 1)
        //remove friend

    nextState = FSM::State::FRIENDS;
}

void FSM::Settings() {
    //LCD_TextOut("Settings")
    //LCD_NextLine()
    //LCD_TextOut("Board Preferences")
    //LCD_NextLine()
    //LCD_TextOut("WIFI")
    //LCD_NextLine()
    //LCD_TextOut("Change Name")
    //LCD_NextLine()
    //LCD_TextOut("Back")
    
    //Button_WaitForResp()

    //if (resp == 1)
        nextState = FSM::State::SETTINGS_BOARDPREFERENCES;
    //if (resp == 2)
        nextState = FSM::State::SETTINGS_WIFI;
    //if (resp == 3)
        nextState = FSM::State::SETTINGS_NAMECHANGE;
    //if (resp == 4)
        nextState = FSM::State::MAIN_MENU;

}

void FSM::SettingsNameChange() {
    //LCD_TextOut("Enter new name:")
    //LCD_NextLine()
    //LCD_TextOut("Back")
    //char newName[100] = BoardKeyBoardFunction(); //Enter letters from sensors, print letters to LCD, middle button to submit

    //confirm 
        //chessServer_setName(response, newName);
        //display resp
        //wait 3 seconds

    //if back 
        //return;
}

void FSM::SettingsBoardPreferences() {
    //Get settings from flash or db

    while(true){
        //LCD_TextOut("Board Preferences")
        //LCD_NextLine()
        //LCD_TextOut("Assist Lights: " + assist light setting (on or off))
        //LCD_NextLine()
        //LCD_TextOut("Sound: " + sound setting)
        //LCD_NextLine()
        //LCD_TextOut("Back")

        //Button_WaitForResp()

        //if (resp == 1)
            //invert setting
            //save settings to flash or db
        //if (resp == 2)
            //invert setting
            //save settings to flash or db
        //if (resp == 3)
            break;
    }

    nextState = FSM::State::SETTINGS;
}

void FSM::SettingsWifi() {
    nextState = FSM::State::SETTINGS;

    //LCD_TextOut("Enter AP Name:")
    //LCD_NextLine()
    //LCD_TextOut("Back")
    //char newAPName[100] = BoardKeyBoardFunction(); //Enter letters from sensors, print letters to LCD, middle button to submit

    //if back 
        //return;

    //LCD_NextLine()
    //LCD_TextOut("Enter AP Name:")
    //LCD_NextLine()
    //LCD_TextOut("Back")
    //char newAPPass[100] = BoardKeyBoardFunction();

    //if back
        //return;

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

    //LCD_TextOut(response)
    //LCD_NextLine()
    //Wait 3 Seconds
}

void FSM::WaitingForPlayer() {
    int retVal = chessServer_awaitTurn();
     //if (retVal == SUCCESS) 
        //LCD_TextOut("Waiting for second player to join")
        //LCD_NextLine()
        //LCD_TextOut("Press any button to cancel")

        //once p2 joins
        //joiningAsPlayer = PlayerColor::WHITE;
        //nextState = FSM::State::INGAME;

        //if button is pressed
            //nextState = FSM::State::MAIN_MENU;

    //if return value == SUCCESS return in game
    //else wait 3 seconds and return waiting on p2
}

void FSM::JoinInviteCreate() {
    //LCD_TextOut("Join a game")
    //LCD_NextLine()
    //LCD_TextOut("Invite friend")
    //LCD_NextLine()
    //LCD_TextOut("Create a game")
    //LCD_NextLine()
    //LCD_TextOut("Back")
    
    //Button_WaitForResp()

    //if (resp == 0)
        nextState = FSM::State::JOIN;
    //if (resp == 1)
        nextState = FSM::State::INVITE;
    //if (resp == 2)
        nextState = FSM::State::CREATE;
    //if (resp == 3)
        nextState = FSM::State::MAIN_MENU;        

}

void FSM::Create() {
     char response[1024];
    //Create game
    //LCD_TextOut(response)
    //Wait 3 seconds
    //if (retVal == SUCCESS) 
        //LCD_TextOut("Waiting for opponent to join")
        //LCD_NextLine()
        //LCD_TextOut("Main Menu     Cancel")

        //once p2 joins
        //joiningAsPlayer = PlayerColor::WHITE;
        //nextState = FSM::State::INGAME;
            //return;

        //if button is pressed
            //if (resp == 1)
                nextState = FSM::State::MAIN_MENU;
            //if (resp == 2)
                //Delete game
                //nextState = FSM::State::JOIN_INVITE_CREATE;
    
}

void FSM::Invite() {
    char response[1024];
    int retVal = chessServer_getFriends(response);
    
    if (retVal == INVALID_RESPONSE || retVal == REQUEST_FAILED) {
        nextState = FSM::State::FRIENDS; 
        return;
    }

    //LCD_TextOut("Friends:")
    //LCD_NextLine()

    if (retVal == NO_FRIENDS) {
        //LCD_TextOut("You currently have no friends to invite")
        //LCD_NextLine()
        //LCD_TextOut("Back")

        //Button_WaitForResp()
        
        nextState = FSM::State::JOIN_INVITE_CREATE;
    }

    flat_vector<std::string, 50> friends = parseFriends(response);

    while(true) {
        //Friends scroll function
        //if selecting a friend to invite
            //LCD_TextOut("Invite 'currentFriendName'?")
            //LCD_NextLine()
            //LCD_TextOut("Yes")
            //LCD_NextLine()
            //LCD_TextOut("No")
            
            //Button_WaitForResp()

            //if (resp == 1)
                char response[1024];
                //Send invite
                //LCD_TextOut(response)
                //Wait 3 seconds for user to read msg
                //if (retVal == SUCCESS) 
                    //LCD_TextOut("Waiting for 'currentFriendName' to join")
                    //LCD_NextLine()
                    //LCD_TextOut("Press any button to cancel")

                    //once p2 joins
                    //joiningAsPlayer = PlayerColor::WHITE;
                    //nextState = FSM::State::IN_GAME;
                    //return;

                    //if button is pressed
                        //Cancel invite
                        //continue;

            //if (resp == 2)
                //continue;

        //if going back 
            break;
    }

    nextState = FSM::State::JOIN_INVITE_CREATE;
}

void FSM::Join() {
    while (true) {
        //LCD_TextOut("Join a Game")
        //LCD_NextLine()
        //LCD_TextOut("Invites")
        //LCD_NextLine()
        //LCD_TextOut("Game Code")
        //LCD_NextLine()
        //LCD_TextOut("Back")

        //Button_WaitForResp()

        //if (resp == 1)
            //LCD_Clear()
            //LCD_TextOut("Invites")
            //LCD_NextLine()

            while (true) {
                //getPendingInvites

                //if error 
                    //Disp for 3 seconds
                    //break;

                //if (# invites != 0)
                    //Scrolling function but for invites

                    //Button_WaitForResp()

                    //if selected invite
                        //LCD_TextOut("Accept 'User's name' Invite?")
                        //LCD_NextLine()
                        //LCD_TextOut("Accept       Decline")
                        //LCD_NextLine()
                        //LCD_TextOut("Back")

                        //Button_WaitForResp()

                        //if (resp == 1)
                            //acceptInvite
                            //if (serverResp == SUCCESS)
                                //joiningAsPlayer = PlayerColor::BLACK;
                                //nextState = FSM::State::INGAME;
                                //return;
                            //else
                                //LCD_TextOut(Response)
                                //Wait 3 seconds
                                //continue;
                        //if (resp == 2)
                            //declineInvite
                            //LCD_TextOut(Response)
                            //Wait 3 seconds
                            //continue;
                        //if (resp == 3)
                            //continue;

                    //if back
                        break;

                //else 
                    //LCD_TextOut("No invites received.")
                    //LCD_NextLine();
                    //LCD_TextOut("Back")

                    //Button_WaitForResp()

                    break;
            
            }

        //if (resp == 2)
            //LCD_Clear()
            //LCD_TextOut("Enter Game Code")
            //LCD_NextLine()
            //LCD_NextLine()
            //LCD_TextOut("Back")

            //char gameCode[10] = BoardKeyBoardFunction(); //Enter characters from sensors, print characters to LCD, middle button to submit
           
            //if submit
                //chessServer_setGameCode(newGameCode)
                //chessServer_joinGame(response)
                //LCD_TextOut(response)
                //Wait 3 seconds
                //if(serverResp == SUCCESS)
                    //joiningAsColor = PlayerColor::BLACK;
                    //nextState = FSM::State::INGAME;
                    //return
                //else
                    continue;

            //if back
                continue;

        //if (resp == 3) 
            break;
    }

    nextState = FSM::State::JOIN_INVITE_CREATE;
}

void FSM::InGame() {
    //-If player's turn, check opponent's move, if move was followed through, initialize boardFSM with awaitLocalMove
    // else, initialize boardFSM with awaitFollowThorugh
    //-If opponent's turn initialize boardFSM with awaitMoveNotice
    
    Board::BoardState initialBoardState;
    bool triggerIncomingMove = false;
    Cell from, to;

    while (true) {
        char serverResp[1024] = chessServer_awaitTurn(serverResp);

        if (serverResp == SUCCESS) {
            char *pt = serverResp + 15; //TODO:change number
            from = Cell(*(strpt + 1) - 96, *(strpt + 2) - 48);
            to = Cell(*(strpt + 3) - 96, *(strpt + 4) - 48);

            //if (magnets_isPieceAt(from)){
                initialBoardState = Board::BoardState::AWAITING_REMOTE_MOVE_NOTICE;
                triggerIncomingMove = true;
            //}

            //else
                initialBoardState = Board::BoardState::AWAITING_LOCAL_MOVE;

            break;
        }

        else if (serverResp == WAITING) {
            initialBoardState = Board::BoardState::AWAITING_REMOTE_MOVE_NOTICE;
            break;
        }

        else 
            continue;
    }

    Board gameBoard(joiningAsColor, initialBoardState);

    if (triggerIncomingMove)
        Board::ReceiveRemoteMove(Move(from, to));
    
    while (true) {
        //LCD_Clear()
        if (gameBoard.boardFSM.GetState() == Board::BoardState::AWAITING_REMOTE_MOVE_NOTICE) {
            //G8RTOS_AddThread(AwaitTurn)

            while (!turnReady) {
                //LCD_TextOut("Waiting for Opponent's Turn...")
                //LCD_NextLine()
                //LCD_NextLine()
                //LCD_TextOut("Board Preferences")
                //LCD_NextLine()
                //LCD_TextOut("Leave Game")

                //if (button_Resp()) {
                    //if (resp == 2)
                        //G8RTOS_RemoveThread(AwaitTurn)
                        //nextState = State::INGAME_BOARDPREFERENCES;
                        //return;
                    //if (resp == 3)
                        //G8RTOS_RemoveThread(AwaitTurn)
                        //nextState = State::LEAVE_GAME;
                        //return;
                //}
            }

            //G8RTOS_RemoveThread(AwaitTurn)

            //TODO:Await turn thread handles remote move and state transition
            continue;
        }

        else if (gameBoard.boardFSM.GetState() == Board::BoardState::AWAITING_LOCAL_MOVE) {
            std::string liftedPieceName = "";
            while (true) {
                //LCD_Clear()
                //LCD_TextOut("Your Turn")
                //LCD_NextLine();
                
                liftedPieceName = gameBoard.GetLiftedPieceName();

                if(liftedPieceName.size() == 0)
                    //LCD_TextOut("No piece chosen")
                else    
                    //LCD_TextOut("Move " + liftedPieceName)

                //LCD_NextLine();
                //LCD_TextOut("Board Preferences")
                //LCD_NextLine();
                //LCD_TextOut("Back")

                //if (button_Resp())
                    //if (resp == 1 && liftedPieceName.size() != 0)
                        //LCD_Clear()
                        //if not valid
                            //say so
                        //LCD_TextOut("Would you like to move your " + liftedPieceName + "?")
                        //LCD_NextLine();
                        //LCD_TextOut("Yes")
                        //LCD_NextLine();
                        //LCD_TextOut("No")

                        //button_waitForResp()

                        //if (resp == 1)
                            //char move[4] = conv to uci
                            //int retVal = chessServer_makeMove(move);

                            //if successful
                                //LCD_Clear()
                                //LCD_TextOut("Sent Move!")
                                gameBoard.SubmitCurrentLocalMove();
                                //wait 3 seconds
                                //return;

                            //else
                                //LCD_Clear()
                                //LCD_TextOut("Something went wrong, try again")
                                //wait 3 seconds
                                //continue;

                        //else if (resp == 2)
                            //continue;

                    //else if (resp == 2)
                        //nextState = State::INGAME_BOARDPREFERENCES;
                        //return;

                    //else if (resp == 3)
                        //nextState = State::LEAVE_GAME;
                        //return;
            }
        }

        else if (gameBoard.boardFSM.GetState() == Board::BoardState::AWAITING_REMOTE_MOVE_FOLLOWTHROUGH) {
            //Get piece name
            //LCD_TextOut("Opponent moved their " + pieceName)
            while(gameBoard.boardFSM.GetState() == Board::BoardState::AWAITING_REMOTE_MOVE_FOLLOWTHROUGH);
        }
    }
    
}

void FSM::InGameBoardPreferences() {
    //TODO: Identical copy of the board pref. in settings, find a way to merge these.
    //Get settings from flash or db

    while(true) {
        //LCD_TextOut("Board Preferences")
        //LCD_NextLine()
        //LCD_TextOut("Assist Lights: " + assist light setting (on or off))
        //LCD_NextLine()
        //LCD_TextOut("Sound: " + sound setting)
        //LCD_NextLine()
        //LCD_TextOut("Back")

        //Button_WaitForResp()

        //if (resp == 1)
            //invert setting
            //save settings to flash or db
        //if (resp == 2)
            //invert setting
            //save settings to flash or db
        //if (resp == 3)
            break;
    }

    nextState = FSM::State::INGAME;
}

void FSM::LeaveGame() {
    //LCD_TextOut("Leave Game?")
    //LCD_NextLine()
    //LCD_TextOut("Yes")
    //LCD_NextLine()
    //LCD_TextOut("No")
    
    //Button_WaitForResp()

    //if (resp == 1)
        //LCD_Clear()
        //LCD_TextOut("Leaving Game...")
        nextState = FSM::State::MAIN_MENU;
        while (true) {
            //deleteGame()
            //if serverResp == SUCCESS
                return;
            //else 
                continue;
        }
}


// HELPER FUNCTIONS

std::string FSM::convertToString(char* ch_a, int length) {
    std::string retString = "";

    for (int i = 0; i < length; i++)
        s += ch_a[i];

    return retString;    
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

        std::string ID[20];
        while (*pt != ','){
            ID += *pt;
            pt++; 
        }
        FSM::friendIDs.push_back(std::stoi(ID));
        pt++;

        std::string name[20];
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

        std::string ID[20];
        while (*pt != ','){
            ID += *pt;
            pt++; 
        }
        FSM::incoming_friendIDs.push_back(std::stoi(ID));
        pt++;

        std::string name[20];
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

        std::string ID[20];
        while (*pt != ','){
            ID += *pt;
            pt++; 
        }
        FSM::outgoing_friendIDs.push_back(std::stoi(ID));
        pt++;

        std::string name[20];
        while (*pt != ','){
            name += *pt;
            pt++; 
        }
        FSM::outgoing_friends.push_back(name);
        pt++;
    }
}
