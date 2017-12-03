#include <stdlib.h>
#include <string.h>
#include "os.h"
#include "ff.h"
#include "sdc.h"

OS_Options_t OS_InitOptions;                           // Inititial Options

OS_GameEntry_t OS_CurrentGame;                         // Path of the currenty/last played game      

OS_State_t OS_CurrState = OS_MAIN_PAGE;                // Current Operatingsystem state
OS_State_t OS_LastState = OS_MAIN_PAGE;                // Last Operatingsystem state

OS_GameEntry_t OS_GameEntries[OS_MAX_NUMBER_OF_GAMES]; // Array containing all game titles & their favorite status
int OS_GamesLoaded = 0;                                // Number of successfully loaded Games


// Compare Function used to sort all game entries
int OS_CmpFunc(const void *a, const void *b)
{
    OS_GameEntry_t game1 = *((OS_GameEntry_t*)a);
    OS_GameEntry_t game2 = *((OS_GameEntry_t*)b);
    int i = -1;

    // Get the first different letters and compare them (case insensitive)
    do
    {
        i++;
        if (game1.Name[i] >= 97 && game1.Name[i] <= 122)
        {
            game1.Name[i] -= 32;
        }
        if (game2.Name[i] >= 97 && game2.Name[i] <= 122)
        {
            game2.Name[i] -= 32;
        }

    } while (game1.Name[i] == game2.Name[i] && game1.Name[i] != '\0');

    return (game1.Name[i] - game2.Name[i]);
}

bool OS_InitializeGameEntries(void)
void OS_LoadOptions(void)
{
    // Load the initial options from internal flash
    // YTBI
}

void OS_UpdateOptions(void)
{
    // Store the current options to internal flash
    // YTBI
}

{
    // If it's a re-initialization reset LoadedGames counter
    if (OS_GamesLoaded != 0) OS_GamesLoaded = 0;

    if (!SDC_Mount()) return false;

    int i = 0;
    DIR directory;
    FILINFO fileInfo;


    // If game-directory couldn't be opened or reading the first file info failed something went wrong
    if (f_opendir(&directory, OS_GAME_DIRECTORY) != FR_OK || f_readdir(&directory, &fileInfo) != FR_OK)
    {
        return false;
    }

    // Get the titles of all games in gamedirectory (If there is no file anymore 0 is written to fname)
    for (; fileInfo.fname[0] != NULL && fileInfo.fattrib != AM_DIR && i < OS_MAX_NUMBER_OF_GAMES; i++)
    {
        copyString(OS_GameEntries[i].Name, fileInfo.fname, OS_MAX_GAME_TITLE_LENGTH + 1);
        OS_GameEntries[i].IsFavorite = false;
        OS_GamesLoaded++;

        if (f_readdir(&directory, &fileInfo) != FR_OK) return false;
    }

    // Close gamedirectory
    f_closedir(&directory);

    // Repeat for favorites
    if (f_opendir(&directory, OS_FAVS_DIRECTORY) != FR_OK || f_readdir(&directory, &fileInfo) != FR_OK)
    {
        return false;
    }

    // Get the titles of all favorites
    for (; fileInfo.fname[0] != NULL && fileInfo.fattrib != AM_DIR && i < OS_MAX_NUMBER_OF_GAMES; i++)
    {
        copyString(OS_GameEntries[i].Name, fileInfo.fname, OS_MAX_GAME_TITLE_LENGTH + 1);
        OS_GameEntries[i].IsFavorite = true;
        OS_GamesLoaded++;

        if (f_readdir(&directory, &fileInfo) != FR_OK) return false;
    }

    // Close directory and sort the list alphabetically (case insensitive)
    f_closedir(&directory);
    qsort(OS_GameEntries, OS_GamesLoaded, sizeof(OS_GameEntry_t), OS_CmpFunc);

    return;
}

void OS_LoadLastPlayed(void)
{
    // The game titles are stored padded by 0bytes (OS_MAX_GAME_TITLE_LENGTH + 1 bytes) followed by '\r''\n'
    int      bufferSize = OS_LAST_PLAYED_GAMES_NUM * (OS_MAX_GAME_TITLE_LENGTH + 3);
    char     buffer[bufferSize];
    uint32_t bytesRead;
    FIL      file;

    // If no SDC can be mounted, file can't be opened/created or (fully) read set gameCounter to 0
    if (!SDC_Mount() || f_open(&file, OS_LAST_PLAYED_FILE, FA_OPEN_EXISTING) != FR_OK ||
        f_read(&file, buffer, bufferSize, &bytesRead) != FR_OK ||
        bytesRead != bufferSize)
    {
        OS_GamesLoaded = 0;
        return;
    }

    // If everything is ok, load the games and close the file afterwards
    int i = 0;
    int currPosition;
    for (; i < OS_LAST_PLAYED_GAMES_NUM; i++)
    {
        currPosition = i * (OS_MAX_GAME_TITLE_LENGTH + 3);

        if (buffer[currPosition] == '\0') break;

        copyString(OS_GameEntries[i].Name, &(buffer[currPosition]), OS_MAX_GAME_TITLE_LENGTH + 1);
        OS_GameEntries[i].IsFavorite = OS_IsFavorite(OS_GameEntries[i].Name);
    }

    OS_GamesLoaded = i;
    f_close(&file);
}

bool OS_UpdateLastPlayed(void)
{
    FIL file;

    // If no SDC can be mounted return false
    if (!SDC_Mount()) return false;

    // If the file doesn't exist try to create a new one, if it does update it
    if (f_open(&file, OS_LAST_PLAYED_FILE, FA_OPEN_EXISTING) != FR_OK)
    {
        // The game titles are stored padded by 0bytes (OS_MAX_GAME_TITLE_LENGTH + 1 bytes) followed by '\r''\n'
        int      bufferSize = OS_LAST_PLAYED_GAMES_NUM * (OS_MAX_GAME_TITLE_LENGTH + 3);
        char     buffer[bufferSize];
        uint32_t bytesWritten;

        // Initialize buffer with the first line (containing first game title) and 0s for the rest
        memset(buffer, '\0', bufferSize);
        appendString(buffer, OS_CurrentGame.Name, OS_MAX_GAME_TITLE_LENGTH + 1);
        buffer[OS_MAX_GAME_TITLE_LENGTH + 2] = '\r';
        buffer[OS_MAX_GAME_TITLE_LENGTH + 3] = '\n';

        // If creating a new file or writing to it fails return false
        if (f_open(&file, OS_LAST_PLAYED_FILE, FA_CREATE_NEW) != FR_OK ||
            f_write(&file, buffer, bufferSize, &bytesWritten) != FR_OK ||
            bytesWritten != bufferSize)
        {
            return false;
        }
        else
        {
            f_close(&file);
            return true;
        }   
    }
    // Update the lastPlayed file
    else
    {
        // The game titles are stored padded by 0bytes (OS_MAX_GAME_TITLE_LENGTH + 1 bytes) followed by '\r''\n'
        int      bufferSize = OS_LAST_PLAYED_GAMES_NUM * (OS_MAX_GAME_TITLE_LENGTH + 3);
        char     buffer[bufferSize];
        uint32_t bytesWritten;
        uint32_t bytesRead;
        int i = 0;

        // Read the lastPlayed entries to buffer
        if (f_read(&file, buffer, bufferSize, &bytesRead) != FR_OK || bytesRead != bufferSize) return false;

        // If current game is already stored as one of the lastPlayed get it's position otherwise the last one
        for (; i < (OS_LAST_PLAYED_GAMES_NUM - 1); i++)
        {
            if (strcmp(&(buffer[i * (OS_MAX_GAME_TITLE_LENGTH + 3)]), OS_CurrentGame.Name) == 0) break;
        }

        // Move all entries 1 position down (last gets deleted)
        // If current game already was in the list start there (-> basically just moves it to the first position)
        int currPosition;
        int lastPosition;
        for (; i > 0; i--)
        {
            currPosition = i * (OS_MAX_GAME_TITLE_LENGTH + 3);
            lastPosition = (i - 1) * (OS_MAX_GAME_TITLE_LENGTH + 3);
            copyString(&(buffer[currPosition]), &(buffer[lastPosition]), OS_MAX_GAME_TITLE_LENGTH + 1);
        }

        // Store current game on position 1 of last played games and write buffer back into OS_LAST_PLAYED_FILE 
        copyString(buffer, OS_CurrentGame.Name, OS_MAX_GAME_TITLE_LENGTH + 1);
        if (f_write(&file, buffer, bufferSize, &bytesWritten) != FR_OK || bytesWritten != bufferSize) return false;
    }

    f_close(&file);
    return true;
}

void OS_LoadFavorites(void)
{
    DIR     directory;
    FILINFO fileInfo;

    // Reset gameCounter
    OS_GamesLoaded = 0;

    // If there is no favorite directory or it can't be opened return
    if (f_opendir(&directory, OS_FAVORITE_DIRECTORY) != FR_OK) return;

    // Store all favorite games (or as many as possible) in OS_GameEntries
    int i = 0;

    do
    {
        if (f_readdir(&directory, &fileInfo) == FR_OK && fileInfo.fattrib != AM_DIR)
        {
            // If the end of the favorite directory is reached stop reading files
            if (fileInfo.fname[0] == NULL) break;

            copyString(OS_GameEntries[i].Name, fileInfo.fname, OS_MAX_GAME_TITLE_LENGTH + 1);
            OS_GameEntries[i].IsFavorite = true;
            OS_GamesLoaded++;
        }
        i++;

    } while (i < OS_MAX_NUMBER_OF_GAMES);

    // Close the directory and sort OS_GameEntries alphabetically (case insensitive)
    f_closedir(&directory);
    qsort(OS_GameEntries, OS_GamesLoaded, sizeof(OS_GameEntry_t), OS_CmpFunc);
}

void OS_UpdateFavorites(OS_GameEntry_t game, OS_Operation_t operation)
{
    // YTBI
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

void OS_GetGamePath(OS_GameEntry_t game, char *path, int pathLength)
{
    if (OS_CurrentGame.IsFavorite)
    {
        copyString(path, OS_FAVORITE_PATH, pathLength);
        appendString(path, OS_CurrentGame.Name, pathLength);
    }
    else
    {
        copyString(path, OS_GAME_PATH, pathLength);
        appendString(path, OS_CurrentGame.Name, pathLength);
    }
}

bool OS_IsFavorite(char *name)
{
    FIL  file;
    int  pathSize = sizeof(OS_FAVORITE_PATH) + OS_MAX_GAME_TITLE_LENGTH + 1;
    char path[pathSize];

    // If the game is located inside the favorite folder it's a favorite
    if (f_open(&file, path, FA_OPEN_EXISTING) == FR_OK)
    {
        f_close(&file);
        return true;
    }
    else
    {
        return false;
    }
}

void OS_DoAction(OS_Action_t action)
{
    OS_State_t temp = OS_CurrState;

    switch (action)
    {
        case OS_SWITCH_TO_PREVIOUS_STATE:
            OS_CurrState = OS_LastState;
            OS_LastState = temp;
            break;

        case OS_SWITCH_TO_STATE_SHOWALL:
            OS_LastState = OS_CurrState;
            OS_CurrState = OS_SHOW_ALL;
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
