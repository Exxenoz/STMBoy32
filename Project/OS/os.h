#ifndef OS_H
#define OS_H

#include "common.h"

#define OS_INIT_OPTIONS_FILE      "init.txt"
#define OS_GAME_DIRECTORY         "Games"
#define OS_FAVS_DIRECTORY         "Games/Favorites"

#define OS_MAX_GAME_TITLE_LENGTH  15
#define OS_MAX_NUMBER_OF_GAMES    1500

#define OS_MAIN_PAGE_BUTTON_LOCK_TIME 150

typedef struct
{
    bool AutoBootCartridge; 
    bool AutoBootSDC;
    bool DrawScaled;
    int  Brightness;
    char lastPlayed[OS_MAX_GAME_TITLE_LENGTH + 1];
}
OS_Options_t;

typedef enum
{
    OS_MAIN_PAGE,
    OS_SHOW_ALL,
    OS_OPTIONS,
    OS_INGAME_FROM_SDC,
    OS_INGAME_FROM_CARTRIDGE,
}
OS_State_t;

typedef struct
{
    char Name[OS_MAX_GAME_TITLE_LENGTH + 1]; // Name of the Game
    bool IsFavorite;                         // Indicates whether game is a favorite or not
}
OS_GameEntry_t;

typedef enum
{
    OS_SWITCH_TO_PREVIOUS_STATE,   // Switch to previous state
    OS_SWITCH_TO_STATE_INGAME_FC,  // Switch current state to OS_INGAME_FROM_CARTRIDGE
    OS_SWITCH_TO_STATE_INGAME_FSD, // Switch current state to OS_INGAME_FROM_SDC
    OS_SWITCH_TO_STATE_SHOWALL,    // Switch current state to OS_SHOW_ALL
    OS_SWITCH_TO_STATE_SHOWFAV,    // Switch current state to OS_SHOW_FAV
    OS_SWITCH_TO_STATE_OPTIONS,    // Switch current state to OS_OPTIONS
    // More TBI
}
OS_Action_t;

extern OS_Options_t OS_InitOptions;

extern OS_GameEntry_t OS_CurrentGame;

extern OS_State_t OS_CurrState;
extern OS_State_t OS_LastState;

extern OS_GameEntry_t OS_GameEntries[OS_MAX_NUMBER_OF_GAMES];
extern int OS_GamesLoaded;

bool OS_InitializeGameEntries(void);
void OS_LoadOptions(void);
void OS_UpdateOptions(void);

void OS_LoadLastPlayed(void);
bool OS_UpdateLastPlayed(void);
OS_GameEntry_t OS_GetGameEntry(char name[OS_MAX_GAME_TITLE_LENGTH + 1]);
void OS_GetGamePath(OS_GameEntry_t game, char* path, int pathLength);
void OS_DoAction(OS_Action_t action);

#endif
