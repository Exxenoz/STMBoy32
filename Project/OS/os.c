#include <stdlib.h>
#include "os.h"
#include "ff.h"
#include "sdc.h"

OS_Options_t OS_InitOptions;                           // Inititial Options

char OS_CurrentGame[OS_MAX_GAME_TITLE_LENGTH + 1];     // Path of the currenty/last played game      

OS_State_t OS_CurrState = OS_MAIN_PAGE;                // Current Operatingsystem state
OS_State_t OS_LastState = OS_MAIN_PAGE;                // Last Operatingsystem state

OS_GameEntry_t OS_GameEntries[OS_MAX_NUMBER_OF_GAMES]; // Array containing all game titles & their favorite status
int OS_GamesLoaded = 0;                                // Number of successfully loaded Games


void OS_LoadInitialOptions(void)
{
    // OS_INIT_OPTIONS_FILE contains (OS_MAX_GAME_TITLE_LENGTH + 1) byte for the name of the last game and
    // 1 addidtional byte for every other information stored in OS_InitOptions
    uint16_t initOptionsBufferSize = (OS_MAX_GAME_TITLE_LENGTH + 1) + 4;
    char     initOptionsBuffer[initOptionsBufferSize];
    uint32_t bytesRead;
    FIL      file;

    // If no SDC can be mounted, file can't be opened/created or (fully) read initialize with default values
    if (!SDC_Mount() || f_open(&file, OS_INIT_OPTIONS_FILE, FA_CREATE_ALWAYS) != FR_OK ||
        f_read(&file, initOptionsBuffer, initOptionsBufferSize, &bytesRead) != FR_OK ||
        bytesRead != initOptionsBufferSize)
    {
        OS_InitOptions.AutoBootCartridge = false;
        OS_InitOptions.AutoBootSDC       = false;
        OS_InitOptions.DrawScaled        = true;
        OS_InitOptions.Brightness        = 100;
        OS_InitOptions.lastPlayed[0]     = '\0';

        return;
    }

    // If everything is ok, load the initial Options values
    OS_InitOptions.AutoBootCartridge = initOptionsBuffer[0] == 0 ? false : true;
    OS_InitOptions.AutoBootSDC       = initOptionsBuffer[1] == 0 ? false : true;
    OS_InitOptions.DrawScaled        = initOptionsBuffer[2] == 0 ? false : true;
    OS_InitOptions.Brightness        = initOptionsBuffer[3] <= 0100 ? initOptionsBuffer[3] : 100;
    copyString(OS_InitOptions.lastPlayed, &(initOptionsBuffer[4]), OS_MAX_GAME_TITLE_LENGTH + 1);

    // Close OS_INIT_OPTIONS_FILE
    f_close(&file);
}

bool OS_StoreOptions(void)
{
    // OS_INIT_OPTIONS_FILE contains (OS_MAX_GAME_TITLE_LENGTH + 1) byte for the name of the last game and
    // 1 addidtional byte for every other information stored in OS_InitOptions
    uint16_t initOptionsBufferSize = (OS_MAX_GAME_TITLE_LENGTH + 1) + 4;
    char     initOptionsBuffer[initOptionsBufferSize];
    uint32_t bytesWritten;
    FIL      file;

    // Convert OS_InitOptions to a storable string (Brightness is increased so it can't be interpreted as '\0'
    initOptionsBuffer[0] = OS_InitOptions.AutoBootCartridge == true ? 1 : 0;
    initOptionsBuffer[1] = OS_InitOptions.AutoBootSDC       == true ? 1 : 0;
    initOptionsBuffer[2] = OS_InitOptions.DrawScaled        == true ? 1 : 0;
    initOptionsBuffer[3] = OS_InitOptions.Brightness;
    copyString(&(initOptionsBuffer[4]), OS_InitOptions.lastPlayed, OS_MAX_GAME_TITLE_LENGTH + 1);
    
    // If no SDC can be mounted, initOptionsFile can't be opened/created or (everything) written to exit
    if (!SDC_Mount() || f_open(&file, OS_INIT_OPTIONS_FILE, FA_CREATE_ALWAYS) != FR_OK ||
        f_write(&file, initOptionsBuffer, initOptionsBufferSize, &bytesWritten) != FR_OK ||
        bytesWritten != initOptionsBufferSize)
    {
        return false;
    }

    // Close OS_INIT_OPTIONS_FILE
    f_close(&file);

    return true;
}

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

bool OS_InitializeGameEntries(void)
{
    if (!SDC_Mount()) return false;

    int i = 0;
    FILINFO fileInfo;


    // If game-directory couldn't be opened or reading the first file info failed something went wrong
    if (f_opendir(&SDC_CurrDir, OS_GAME_DIRECTORY) != FR_OK || f_readdir(&SDC_CurrDir, &fileInfo) != FR_OK)
    {
        return false;
    }

    // Get the titles of all games in gamedirectory (If there is no file anymore 0 is written to fname)
    for (; fileInfo.fname[0] != NULL && i < OS_MAX_NUMBER_OF_GAMES; i++)
    {
        if (f_readdir(&SDC_CurrDir, &fileInfo) != FR_OK) return false;

        copyString(OS_GameEntries[i].Name, fileInfo.fname, OS_MAX_GAME_TITLE_LENGTH + 1);
        OS_GameEntries[i].IsFavorite = false;
        OS_GamesLoaded++;
    }

    // Repeat for favorites (dont close game directory since Favorite directory should be located inside it)
    if (f_opendir(&SDC_CurrDir, OS_FAVS_DIRECTORY) != FR_OK || f_readdir(&SDC_CurrDir, &fileInfo) != FR_OK)
    {
        return false;
    }

    // Get the titles of all favorites
    for (; fileInfo.fname[0] != NULL && i < OS_MAX_NUMBER_OF_GAMES; i++)
    {
        if (f_readdir(&SDC_CurrDir, &fileInfo) != FR_OK) return false;

        copyString(OS_GameEntries[i].Name, fileInfo.fname, OS_MAX_GAME_TITLE_LENGTH + 1);
        OS_GameEntries[i].IsFavorite = true;
        OS_GamesLoaded++;
    }

    // Sort the list alphabetically (case insensitive) and close directories
    qsort(OS_GameEntries, OS_GamesLoaded, sizeof(OS_GameEntry_t), OS_CmpFunc);
    f_closedir(&SDC_CurrDir);

    return true;
}

OS_GameEntry_t OS_GetGameEntry(char name[OS_MAX_GAME_TITLE_LENGTH + 1])
{
    int c;

    for (int i = 0; i < OS_GamesLoaded; i++)
    {
        for (c = 0; c < OS_MAX_GAME_TITLE_LENGTH; c++)
        {
            if (OS_GameEntries[i].Name[c] != name[c]) break;
        }

        // If the previous loop didn't break the names match
        if (c == OS_MAX_GAME_TITLE_LENGTH) return OS_GameEntries[i];
    }

    // If no match was found return empty object
    OS_GameEntry_t noMatch;
    noMatch.Name[0]    = '\0';
    noMatch.IsFavorite = false;

    return noMatch;
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
