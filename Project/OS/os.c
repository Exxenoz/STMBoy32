#include <stdlib.h>
#include "os.h"
#include "ff.h"
#include "sdc.h"

OS_State_t OS_CurrState = OS_MAIN_PAGE;                // Current Operatingsystem state
OS_State_t OS_LastState = OS_MAIN_PAGE;                // Last Operatingsystem state

OS_GameEntry_t OS_GameEntries[OS_MAX_NUMBER_OF_GAMES]; // Array containing all game titles & their favorite status
int OS_GamesLoaded = 0;                                // Number of successfully loaded Games


// Compare Function used to sort all game entries
int OS_CmpFunc(const void *a, const void *b)
{
    char letter1 = *((char*)a);
    char letter2 = *((char*)b);

    // Compare case insensitive
    if (letter1 >= 97 && letter1 <= 122)
    {
        letter1 -= 32;
    }

    if (letter2 >= 97 && letter2 <= 122)
    {
        letter2 -= 32;
    }

    return (letter1 - letter2);
}

bool OS_GetAllGameEntries(void)
{
    int i = 0;
    FILINFO fileInfo;

    // If game-directory couldn't be opened or reading the first file info failed something went wrong
    if (f_opendir(&SDC_CurrDir, OS_GAME_DIRECTORY) == FR_OK && f_readdir(&SDC_CurrDir, &fileInfo) == FR_OK)
    {
        return false;
    }

    // Get the titles of all games in gamedirectory (If there is no file anymore 0 is written to fname)
    for (; fileInfo.fname[0] != NULL && i < OS_MAX_NUMBER_OF_GAMES; i++)
    {
        if (f_readdir(&SDC_CurrDir, &fileInfo) != FR_OK) return false;

        copyString(OS_GameEntries[i].Name, fileInfo.fname, OS_MAX_GAME_TITLE_LENGTH);
        OS_GameEntries[i].IsFavorite = false;
        OS_GamesLoaded++;
    }

    // Repeat for favorites (dont close game directory since Favorite directory should be located inside it)
    if (f_opendir(&SDC_CurrDir, OS_FAVS_DIRECTORY) == FR_OK && f_readdir(&SDC_CurrDir, &fileInfo) == FR_OK)
    {
        return false;
    }

    // Get the titles of all favorites
    for (; fileInfo.fname[0] != NULL && i < OS_MAX_NUMBER_OF_GAMES; i++)
    {
        if (f_readdir(&SDC_CurrDir, &fileInfo) != FR_OK) return false;

        copyString(OS_GameEntries[i].Name, fileInfo.fname, OS_MAX_GAME_TITLE_LENGTH);
        OS_GameEntries[i].IsFavorite = true;
        OS_GamesLoaded++;
    }

    // Sort the list alphabetically (case insensitive)
    qsort(OS_GameEntries, OS_GamesLoaded, sizeof(OS_GameEntry_t), OS_CmpFunc);

    return true;
}

void OS_DoAction(OS_Action_t action)
{
    switch (action)
    {
        case OS_SWITCH_TO_STATE_SHOWALL:
            OS_LastState = OS_CurrState;
            OS_CurrState = OS_SHOW_ALL;
            break;
        case OS_SWITCH_TO_STATE_SHOWFAV:
            OS_LastState = OS_CurrState;
            OS_CurrState = OS_SHOW_FAV;
            break;
        case OS_SWITCH_TO_STATE_OPTIONS:
            OS_LastState = OS_CurrState;
            OS_CurrState = OS_OPTIONS;
            break;
        case OS_SWITCH_TO_STATE_INGAME_FC:
            OS_LastState = OS_CurrState;
            OS_CurrState = OS_INGAME_FROM_CARTRIDGE;
            break;
        case OS_SWITCH_TO_STATE_INGAME_FSD:
            OS_LastState = OS_CurrState;
            OS_CurrState = OS_INGAME_FROM_SDC;
            break;
    }
}
