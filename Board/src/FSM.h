#pragma once

#include <cstdio>
#include <string>
#include <iostream>
#include "flat_vector.h"

extern "C" {
    #include "chessServer.h"
    #include "G8RTOS_Semaphores.h"
    #include "G8RTOS_CriticalSection.h"
};

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

        State curState;
        State nextState;

        PlayerColor joiningAsColor;

        bool turnReady;

        std::string convertToString();
        flat_vector<std::string, 50> parseFriends(char* response);

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