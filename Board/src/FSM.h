#pragma once

#include <cstdio>
#include <string>
#include <iostream>
#include "flat_vector.h"

extern "C" {
    #include "chessServer.h"
    #include "G8RTOS_Scheduler.h"
    #include "G8RTOS_Semaphores.h"
    #include "G8RTOS_CriticalSection.h"
    #include <driverlib.h>
    #include "BSP.h"
    #include "demo_sysctl.h"
    #include "msp_compatibility.h"
};

#include "ChessBoard.h"
#include "LCD_CharacterDisplay.h"
#include "Menu.h"
#include "Friend.h"

namespace RemoteChess {
    class FSM {
        public:
        enum class State {
              INITIAL_CONNECTION
            , INITIAL_WIFI_CHANGE
            , DOWNLOAD_CURRENT_GAME
            , MAIN_MENU
            , FRIENDS
            , FRIENDS_ADD
            , FRIENDS_SELECT
            , FRIENDS_SELECT_INVITE
            , FRIENDS_SELECT_REMOVE
            , SETTINGS
            , SETTINGS_BOARDPREFERENCES
            , SETTINGS_WIFI
            , SETTINGS_NAMECHANGE
            , RESIGN
            , CREATE
            , CPU_GAME
            , JOIN
            , JOIN_CREATE
            , INCOMING_INVITE
            , NOGAME
            , INGAME
            , WINNER
            , LOSER
            , OPPONENT_RESIGNED
        };

        private:
        State curState;
        State nextState;
        bool didChangeState = false;

        PlayerColor joiningAsColor;
        LCD_CharacterDisplay lcd;
        Menu menu;
        
        bool turnReady;
        bool isGameRunning = false;
        bool isCPUgame = false;

        const uint16_t LCD_DISPLAY_TIME = 2000;
        const uint16_t SERVER_PING_DELAY = 3000;
        
        public:
        FSM(State initialState = State::INITIAL_CONNECTION) : curState(initialState) { }

        const Friend* currentFriend;
       
        flat_vector<Friend, 10> friends; 
        flat_vector<Friend, 10> incomingFriends;
        flat_vector<Friend, 10> outgoingFriends;

        void parseFriends(char* response);
        void parseInvites(char* response);

        void InitialConnection();
        void InitialWIFIChange();
        void DownloadCurrentGame();
        void Main_Menu();
        void Friends();
        void FriendsAdd();
        void FriendsSelect();
        void FriendsSelectInvite();
        void FriendsSelectRemove();
        void Settings();
        // void SettingsBoardPreferences();
        // void SettingsWifi();
        // void SettingsNameChange();
        void JoinCreate();
        void Create();
        void CPUGame();
        void Join();
        void IncomingInvite();
        void NoGame();
        void InGame();
        void Resign();
        // void InGameBoardPreferences();
        // void LeaveGame();

        public:
        
        void FSMController();
        State GetState() const { return curState; };
    };
}
