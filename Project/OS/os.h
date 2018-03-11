#ifndef OS_H
#define OS_H

#include "common.h"
#include "error.h"

#define OS_LAST_PLAYED_GAMES_NUM      5                     // defines how many of the last played games are stored

#define OS_LAST_PLAYED_FILE           "lastPlayed.txt"
#define OS_GAME_DIRECTORY             "Games"
#define OS_GAME_PATH                  "Games/"
#define OS_FAVORITE_DIRECTORY         "Games/Favorites"
#define OS_FAVORITE_PATH              "Games/Favorites/"

#define OS_MAX_GAME_TITLE_LENGTH      18
#define OS_MAX_NUMBER_OF_GAMES        100                   // defines how many games can be loaded to buffer
#define OS_MAX_PATH_LENGTH            ((sizeof(OS_FAVORITE_PATH) + OS_MAX_GAME_TITLE_LENGTH + 1))

#define OS_MAIN_PAGE_BUTTON_LOCK_TIME 150

typedef struct
{
    char Name[OS_MAX_GAME_TITLE_LENGTH + 1];
    bool IsFavorite;
}
OS_GameEntry_t;

typedef struct
{
    bool AutoBootCartridge;
    bool AutoBootSDC;
    bool DrawScaled;
    int  Brightness;
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

typedef enum
{
    OS_NO_ACTION,
    OS_SWITCH_TO_PREVIOUS_STATE,   // Switch to previous state
    OS_SWITCH_TO_STATE_INGAME_FC,  // Switch current state to OS_INGAME_FROM_CARTRIDGE
    OS_SWITCH_TO_STATE_INGAME_FSD, // Switch current state to OS_INGAME_FROM_SDC
    OS_SWITCH_TO_STATE_SHOWALL,    // Switch current state to OS_SHOW_ALL
    OS_SWITCH_TO_STATE_OPTIONS,    // Switch current state to OS_OPTIONS
    // More TBI
}
OS_Action_t;

typedef struct
{
    char buffer[OS_LAST_PLAYED_GAMES_NUM * (OS_MAX_GAME_TITLE_LENGTH + 2) + 1];
}
test_t;

extern OS_Options_t OS_InitOptions;

extern OS_GameEntry_t OS_CurrentGame;

extern OS_State_t OS_CurrState;
extern OS_State_t OS_LastState;

extern OS_GameEntry_t OS_GameEntries[OS_MAX_NUMBER_OF_GAMES];
extern int OS_LoadedGamesCounter;
extern int OS_TotalGamesCounter;


void OS_LoadOptions(void);
void OS_UpdateOptions(void);

Error_Def_t OS_LoadGameEntries(char *startingName, bool previous, bool onlyFavorites);
Error_Def_t OS_InvertFavoriteStatus(OS_GameEntry_t *p_game);
Error_Def_t OS_LoadLastPlayed(void);
Error_Def_t OS_UpdateLastPlayed(void);

Error_Def_t OS_GetGameEntry(char *name, OS_GameEntry_t **gameEntry);
Error_Def_t OS_GetGamePath(OS_GameEntry_t *p_game, char *path, int pathLength);
Error_Def_t OS_IsFavorite(OS_GameEntry_t *p_game);
Error_Def_t OS_RemoveGameEntry(int currGameEntryIndex);
void OS_DoAction(OS_Action_t action);

#endif
