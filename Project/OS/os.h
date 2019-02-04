#ifndef OS_H
#define OS_H

#include "common.h"
#include "error.h"
#include "cmod.h"
#include "sdc.h"
#include "ff.h"


#define OS_LAST_PLAYED_GAMES_NUM          5                     // defines how many of the last played games are stored
#define OS_NUMBER_OF_SUPPORTED_LANGUAGES  1

#define OS_LAST_PLAYED_FILE               "lastPlayed.txt"
#define OS_OPTIONS_FILE                   "options.txt"
#define OS_GAME_DIRECTORY                 "Games"
#define OS_GAME_PATH                      "Games/"
#define OS_SAVEGAME_DIRECTORY             "Games"           // ToDo: Create SaveGames folder on SDC
#define OS_SAVEGAME_PATH                  "Games/"
#define OS_SAVEGAME_SUFFIX                ".sav"
#define OS_FAVORITE_DIRECTORY             "Games/Favorites"
#define OS_FAVORITE_PATH                  "Games/Favorites/"

#define OS_MAX_GAME_TITLE_LENGTH          18
#define OS_MAX_NUMBER_OF_GAMES            100                    // defines how many games can be loaded to buffer
#define OS_MAX_PATH_LENGTH                ((sizeof(OS_FAVORITE_PATH) + OS_MAX_GAME_TITLE_LENGTH + 1))

#define OS_MAIN_PAGE_BUTTON_LOCK_TIME     400
#define OS_OPTIONS_PAGE_BUTTON_LOCK_TIME  400

#define OS_MIN_BRIGHTNESS                 3                      // User can't go below 3%.

typedef enum
{
    OS_MAINPAGE,
    OS_SHOW_ALL,
    OS_OPTIONS,
    OS_INGAME_FROM_SDC,
    OS_INGAME_FROM_CARTRIDGE,
}
OS_State_t;

// Has to start at 0 and mustn't skip any values.
// Mustn't define more than OS_NUMBER_OF_SUPPORTED_LANGUAGES values.
typedef enum
{
    OS_ENGLISH = 0,
}
OS_Language_t;

typedef enum
{
    OS_NO_ACTION,
    OS_SWITCH_TO_PREVIOUS_STATE,   // Switch to previous state.
    OS_SWITCH_TO_STATE_MAINPAGE,   // Switch current state to OS_MAINPAGE.
    OS_SWITCH_TO_STATE_INGAME_FC,  // Switch current state to OS_INGAME_FROM_CARTRIDGE.
    OS_SWITCH_TO_STATE_INGAME_FSD, // Switch current state to OS_INGAME_FROM_SDC.
    OS_SWITCH_TO_STATE_SHOWALL,    // Switch current state to OS_SHOW_ALL.
    OS_SWITCH_TO_STATE_OPTIONS,    // Switch current state to OS_OPTIONS.
    OS_TOGGLE_OPTION_DIRECT_BOOT,
    OS_TOGGLE_OPTION_SCALING,
    OS_SAVE_CARTRIDGE,
    OS_EDIT_BRIGHTNESS,
    OS_EDIT_LANGUAGE,
}
OS_Action_t;


#pragma pack(1)
typedef struct
{
    char Name[OS_MAX_GAME_TITLE_LENGTH + 1];
    bool IsFavorite;
}
OS_GameEntry_t;

#pragma pack(1)
typedef struct
{
    int           Brightness;
    bool          AutoBoot;
    bool          DrawScaled;
    OS_Language_t Language;
}
OS_Options_t;


extern OS_GameEntry_t  OS_CurrentGame;

extern OS_State_t      OS_CurrState;
extern OS_State_t      OS_LastState;

extern int             OS_LoadedGamesCounter;
extern int             OS_TotalGamesCounter;

extern bool            OS_EditBrightnessMode;
extern bool            OS_EditLanguageMode;

extern OS_GameEntry_t  OS_GameEntries[OS_MAX_NUMBER_OF_GAMES];

extern OS_Options_t    OS_Options;



Error_Def_t OS_LoadLastPlayed(void);
Error_Def_t OS_LoadGameEntries(char *startingName, bool previous, bool onlyFavorites);
Error_Def_t OS_RemoveGameEntry(int currGameEntryIndex);
Error_Def_t OS_UpdateOptions(void);
Error_Def_t OS_UpdateLastPlayed(void);

Error_Def_t OS_GetGamePath(OS_GameEntry_t *p_game, char *path, int pathLength);
Error_Def_t OS_GetSaveGamePath(OS_GameEntry_t *p_game, char *path, int pathLength);

Error_Def_t OS_Set_IsFavorite(OS_GameEntry_t *p_game);
Error_Def_t OS_InvertFavoriteStatus(OS_GameEntry_t *p_game);

void OS_DoAction(OS_Action_t action);

#endif
