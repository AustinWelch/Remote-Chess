#pragma once

#include <cstdio>
#include <string>
#include <iostream>
#include "flat_vector.h"

extern "C" {
    #include "chessServer.h"
    #include "G8RTOS_Semaphores.h"
    #include "G8RTOS_CriticalSection.h"
    #include <driverlib.h>
    #include "BSP.h"
    #include "demo_sysctl.h"
    #include "msp_compatibility.h"
};

#include "ChessBoard.h"

namespace RemoteChess {
    class FSM {
        enum class State {
              INITIAL_CONNECTION
            , INITIAL_WIFI_CHANGE
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
            , FIND_GAME
            , WAITING_ON_P2
            , JOIN_INVITE_CREATE
            , CREATE
            , INVITE
            , JOIN
            , INGAME
            , INGAME_BOARDPREFERENCES
            , LEAVE_GAME
        };

        FSM() : curState(State::INITIAL_CONNECTION) { }

        std::string currentFriendName;
        int currentFriendID;

        flat_vector<std::string, 50> friends;
        flat_vector<std::string, 50> incoming_friends;
        flat_vector<std::string, 50> outgoing_friends;
        flat_vector<int, 50> friendIDs;
        flat_vector<int, 50> incoming_friendIDs;
        flat_vector<int, 50> outgoing_friendIDs;

        flat_vector<int, 50> inviterIDs;
        flat_vector<int, 50> inviteGameCode;
        flat_vector<std::string, 50> inviterNames;

        State curState;
        State nextState;

        PlayerColor joiningAsColor;

        bool turnReady;

        std::string convertToString(char* ch_a, int length);
        void convertToChar(std::string str, char* out);
        void parseFriends(char* response);
        void parseInvites(char* response);

        void InitialConnection();
        void InitialWIFIChange();
        void Main_Menu();
        void Friends();
        void FriendsAdd();
        void FriendsSelect();
        void FriendsSelectInvite();
        void FriendsSelectRemove();
        void Settings();
        void SettingsBoardPreferences();
        void SettingsWifi();
        void SettingsNameChange();
        void FindGame();
        void WaitingForPlayer();
        void JoinInviteCreate();
        void Create();
        void Invite();
        void Join();
        void InGame();
        void InGameBoardPreferences();
        void LeaveGame();

        public:
        
        void FSMController();
    };
}
