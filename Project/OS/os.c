#include <stdlib.h>
#include <string.h>
#include "os.h"
#include "ff.h"
#include "sdc.h"
#include "error.h"

OS_Options_t OS_InitOptions;                           // Inititial Options

OS_GameEntry_t OS_CurrentGame;                         // Path of the currenty/last played game      

OS_State_t OS_CurrState = OS_MAIN_PAGE;                // Current Operatingsystem state
OS_State_t OS_LastState = OS_MAIN_PAGE;                // Last Operatingsystem state

OS_GameEntry_t OS_GameEntries[OS_MAX_NUMBER_OF_GAMES]; // Array containing all game titles & their favorite status
int OS_LoadedGamesCounter = 0;                         // Number of successfully loaded Games
int OS_TotalGamesCounter = 0;                          // Number of all games detected on SDC


// Compare Function used to sort all game entries
int OS_CmpGameEntries(const void *a, const void *b)
{
    OS_GameEntry_t game1 = *((OS_GameEntry_t*)a);
    OS_GameEntry_t game2 = *((OS_GameEntry_t*)b);

    return CmpStrings(game1.Name, game2.Name);
}

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

// Loads maxLoad gameentries in alphabetical (starting with the one following startingName) order to OS_GameEntries
uint32_t OS_LoadGameEntries(char* startingName, bool previous, bool onlyFavorites)
{
    DIR     directory;
    char    *directoryPath;
    bool    isFavorite;
    FILINFO fileInfo;

    // Clear OS_GameEntries
    memset(OS_GameEntries, 0, OS_MAX_NUMBER_OF_GAMES * sizeof(OS_GameEntry_t));

    // Reset gameCounters
    OS_LoadedGamesCounter = 0;
    OS_TotalGamesCounter  = 0;

    // If only favorites are to be read set the directory-path accordingly
    if (onlyFavorites)
    {
        directoryPath = OS_FAVORITE_DIRECTORY;
        isFavorite    = true;
    }
    else
    {
        directoryPath = OS_GAME_DIRECTORY;
        isFavorite    = false;
    }

    // If no SDC can be mounted or the specified directory doesn't exist / can't be opened return
    if (!SDC_Mount())
    {
        return SDC_ERROR_NO_SDC;
    }

    if (f_opendir(&directory, directoryPath) != FR_OK)
    {
        return SDC_ERROR_GAME_DIR_ACCESS_FAILED;
    }

    // Get the alphabetically 'lowest' OS_MAX_NUMBER_OF_GAMES titles 'higher' than startingName from the directory
    bool finishedLoading = false;
    do
    {
        if (f_readdir(&directory, &fileInfo) == FR_OK && fileInfo.fname[0] != NULL && fileInfo.fattrib != AM_DIR)
        {
            // If the read gametitle is not 'higher'/'lower' than startingName continue
            if (previous && CmpStrings(startingName, fileInfo.fname) < 0) continue;
            if (!previous && CmpStrings(startingName, fileInfo.fname) > 0) continue;

            // If OS_GameEntries is not fully populated add the name of the game unconditionally
            if (OS_LoadedGamesCounter < OS_MAX_NUMBER_OF_GAMES)
            {
                CopyString(OS_GameEntries[OS_LoadedGamesCounter].Name, fileInfo.fname, OS_MAX_GAME_TITLE_LENGTH + 1);
                OS_GameEntries[OS_LoadedGamesCounter].IsFavorite = isFavorite;
                OS_LoadedGamesCounter++;
            }
            // If it is, check if the current title is alphabetically 'lower' than any of the stored titles
            else
            {
                int currentDifferenze;
                int biggestDifferenze = 0;
                int highestTitleIndex = -1;

                for (int i = 0; i < OS_MAX_NUMBER_OF_GAMES; i++)
                {
                    currentDifferenze = CmpStrings(OS_GameEntries[i].Name, fileInfo.fname);
                    if (currentDifferenze > biggestDifferenze)
                    {
                        biggestDifferenze = currentDifferenze;
                        highestTitleIndex  = i;
                    }
                }

                // If it is, replace the alphabetically 'highest' stored title with the currently read title
                if (highestTitleIndex != -1)
                {
                    CopyString(OS_GameEntries[highestTitleIndex].Name, fileInfo.fname, OS_MAX_GAME_TITLE_LENGTH + 1);
                }
            }

            // Increase total gameCounter after every successful read even if the game title doesn't get stored
            OS_TotalGamesCounter++;
        }

        // If fileInfo.fname[0] == NULL (all titles read) and the favorite directory was read loading finished
        // Loading the favorite gametitles is always the last thing to do whether we read just them or all titles
        if (fileInfo.fname[0] == NULL)
        {
            if (isFavorite)
            {
                finishedLoading = true;
            }
            else
            {
                directoryPath = OS_FAVORITE_DIRECTORY;
                isFavorite    = true;

                // Close the game-directory
                f_closedir(&directory);

                // If opening the favorite directory fails sort what was already stored and exit
                if (f_opendir(&directory, directoryPath) != FR_OK)
                {
                    qsort(OS_GameEntries, OS_LoadedGamesCounter, sizeof(OS_GameEntry_t), OS_CmpGameEntries);
                    return SDC_ERROR_FAV_DIR_ACCESS_FAILED;
                }
            }
        }

    } while (!finishedLoading);

    // Close directory and sort the list alphabetically (case insensitive)
    f_closedir(&directory);
    qsort(OS_GameEntries, OS_LoadedGamesCounter, sizeof(OS_GameEntry_t), OS_CmpGameEntries);
    return OS_SUCCESS;
}

uint32_t OS_InvertFavoriteStatus(OS_GameEntry_t *p_game)
{
    // Define path arrays with maximal needed size
    char     oldPath[OS_MAX_PATH_LENGTH];
    char     newPath[OS_MAX_PATH_LENGTH];
    uint32_t err_code;

    // Get old path
    err_code = OS_GetGamePath(p_game, oldPath, OS_MAX_PATH_LENGTH);
    ERROR_CHECK(err_code);

    // Invert the favorite attribute
    p_game->IsFavorite = p_game->IsFavorite ? false : true;

    // Get the new path
    if (p_game->IsFavorite)
    {
        CopyString(newPath, OS_FAVORITE_PATH, OS_MAX_PATH_LENGTH);
        AppendString(newPath, p_game->Name, OS_MAX_PATH_LENGTH);
    }
    else
    {
        CopyString(newPath, OS_GAME_PATH, OS_MAX_PATH_LENGTH);
        AppendString(newPath, p_game->Name, OS_MAX_PATH_LENGTH);
    }

    // Move the game into the new directory
    f_rename(oldPath, newPath);
    return OS_SUCCESS;
}

uint32_t OS_LoadLastPlayed(void)
{
    // The game titles are stored padded by 0bytes (OS_MAX_GAME_TITLE_LENGTH + 1 bytes) followed by '\r''\n'
    FIL      file;  
    int      bufferSize = OS_LAST_PLAYED_GAMES_NUM * (OS_MAX_GAME_TITLE_LENGTH + 3);
    char     buffer[bufferSize];
    uint32_t bytesRead;
    uint32_t err_code;

    // Clear OS_GameEntries
    memset(OS_GameEntries, 0, OS_MAX_NUMBER_OF_GAMES * sizeof(OS_GameEntry_t));

    // If no SDC can be mounted, file can't be opened/created or (fully) read set gameCounter to 0
    if (!SDC_Mount())
    {
        OS_LoadedGamesCounter = 0;
        return SDC_ERROR_NO_SDC;
    }

    if (f_open(&file, OS_LAST_PLAYED_FILE, FA_OPEN_EXISTING | FA_READ) != FR_OK ||
        f_read(&file, buffer, bufferSize, &bytesRead) != FR_OK ||
        bytesRead != bufferSize)
    {
        OS_LoadedGamesCounter = 0;
        return SDC_ERROR_FILE_ACCESS_FAILED;
    }

    // If everything is ok close the file, load the games and set gameCounter accordingly
    f_close(&file);
    int  currPosition = 0;
    int  i = 0;

    for (; i < OS_LAST_PLAYED_GAMES_NUM; i++, currPosition += (OS_MAX_GAME_TITLE_LENGTH + 3))
    {        
        if (buffer[currPosition] == '\0') break;

        CopyChars(OS_GameEntries[i].Name, &(buffer[currPosition]), OS_MAX_GAME_TITLE_LENGTH + 1);
        err_code = OS_IsFavorite(&(OS_GameEntries[i]));
        ERROR_CHECK(err_code);
    }

    OS_LoadedGamesCounter = i;
    OS_TotalGamesCounter  = i;
    return OS_SUCCESS;
}

uint32_t OS_UpdateLastPlayed(void)
{
    FIL file;

    // The game titles are stored padded by 0bytes (OS_MAX_GAME_TITLE_LENGTH + 1 bytes) followed by '\r''\n'
    int      bufferSize = OS_LAST_PLAYED_GAMES_NUM * (OS_MAX_GAME_TITLE_LENGTH + 3);
    char     buffer[bufferSize];
    uint32_t bytesWritten;
    uint32_t bytesRead;

    // If no SDC can be mounted return false
    if (!SDC_Mount())
    {
        return SDC_ERROR_NO_SDC;
    }

    // If the file doesn't exist try to create a new one, if it does update it
    if (f_open(&file, OS_LAST_PLAYED_FILE, FA_OPEN_EXISTING | FA_READ | FA_WRITE) != FR_OK)
    {
        // Initialize buffer with the first line (containing first game title) and 0s for the rest
        memset(buffer, '\0', bufferSize);
        CopyString(buffer, OS_CurrentGame.Name, OS_MAX_GAME_TITLE_LENGTH + 1);
        buffer[OS_MAX_GAME_TITLE_LENGTH + 1] = '\r';
        buffer[OS_MAX_GAME_TITLE_LENGTH + 2] = '\n';

        // If creating a new file or writing to it fails return false
        if (f_open(&file, OS_LAST_PLAYED_FILE, FA_CREATE_NEW | FA_WRITE) != FR_OK ||
            f_write(&file, buffer, bufferSize, &bytesWritten) != FR_OK ||
            bytesWritten != bufferSize)
        {
            return SDC_ERROR_FILE_ACCESS_FAILED;
        }
        else
        {
            f_close(&file);
            return OS_SUCCESS;
        }   
    }
    // Update the lastPlayed file
    else
    {
        int i = 0;

        // Read the lastPlayed entries to buffer
        if (f_read(&file, buffer, bufferSize, &bytesRead) != FR_OK)
        {
            return SDC_ERROR_FILE_ACCESS_FAILED;
        }

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

            CopyChars(&(buffer[currPosition]), &(buffer[lastPosition]), OS_MAX_GAME_TITLE_LENGTH + 3);
        }

        // Get the string representing the current game
        char paddedName[OS_MAX_GAME_TITLE_LENGTH + 3];
        memset(paddedName, 0, sizeof(paddedName));

        CopyString(paddedName, OS_CurrentGame.Name, OS_MAX_GAME_TITLE_LENGTH + 1);
        paddedName[OS_MAX_GAME_TITLE_LENGTH + 1] = '\r';
        paddedName[OS_MAX_GAME_TITLE_LENGTH + 2] = '\n';
        
        CopyChars(buffer, paddedName, OS_MAX_GAME_TITLE_LENGTH + 3);

        // Store current game on position 1 of last played games and write buffer back into OS_LAST_PLAYED_FILE
        f_lseek(&file, 0);
        if (f_write(&file, buffer, bufferSize, &bytesWritten) != FR_OK || bytesWritten != bufferSize)
        {
            return SDC_ERROR_FILE_ACCESS_FAILED;
        }
    }

    f_close(&file);
    return OS_SUCCESS;
}

uint32_t OS_GetGameEntry(char *name, OS_GameEntry_t **gameEntry)
{
    for (int i = 0; i < OS_LoadedGamesCounter; i++)
    {
        if (CmpStrings(OS_GameEntries[i].Name, name) == 0)
        {
            *gameEntry = &(OS_GameEntries[i]);
            return OS_SUCCESS;
        }
    }

    return OS_ERROR_GAME_NOT_FOUND;
}

uint32_t OS_RemoveGameEntry(int currGameEntryIndex)
{
    if (currGameEntryIndex >= OS_LoadedGamesCounter)
    {
        return OS_ERROR_INVALID_INDEX;
    }

    for (int i = currGameEntryIndex; i < OS_LoadedGamesCounter; i++)
    {
        if (i == OS_LoadedGamesCounter - 1)
        {
            CopyString(OS_GameEntries[i].Name, "", OS_MAX_GAME_TITLE_LENGTH + 1);
        }
        else
        {
            CopyString(OS_GameEntries[i].Name, OS_GameEntries[i + 1].Name, OS_MAX_GAME_TITLE_LENGTH + 1);
            OS_GameEntries[i].IsFavorite = OS_GameEntries[i + 1].IsFavorite;
        }
    }

    OS_LoadedGamesCounter--;
    OS_TotalGamesCounter--;
    return OS_SUCCESS;
}

uint32_t OS_GetGamePath(OS_GameEntry_t *p_game, char *path, int pathLength)
{
    // Check if the array in which the path is to be stored is long enough
    if (pathLength < OS_MAX_PATH_LENGTH)
    {
        return OS_ERROR_INVALID_PATH_LENGTH;
    }

    FIL  file;

    // Check if the game is located inside the favorite folder
    CopyString(path, OS_FAVORITE_PATH, pathLength);
    AppendString(path, p_game->Name, pathLength);
    if (f_open(&file, path, FA_OPEN_EXISTING) == FR_OK)
    {
        f_close(&file);
        return OS_SUCCESS;
    }

    // If the game isn't located inside the favorite folder check if its located in the game folder
    CopyString(path, OS_GAME_PATH, pathLength);
    AppendString(path, p_game->Name, pathLength);
    if (f_open(&file, path, FA_OPEN_EXISTING) == FR_OK)
    {
        f_close(&file);
        return OS_SUCCESS;
    }

    // If it's located in neither it doesn't exist
    memset(path, 0, pathLength);
    return OS_ERROR_GAME_NOT_FOUND;
}

uint32_t OS_IsFavorite(OS_GameEntry_t *p_game)
{
    char     path[OS_MAX_PATH_LENGTH];
    char     favoritePath[OS_MAX_PATH_LENGTH];
    uint32_t err_code;

    err_code = OS_GetGamePath(p_game, path, OS_MAX_PATH_LENGTH);
    ERROR_CHECK(err_code);

    CopyString(favoritePath, OS_FAVORITE_PATH, OS_MAX_PATH_LENGTH);
    AppendString(favoritePath, p_game->Name, OS_MAX_PATH_LENGTH);

    if (CmpStrings(path, favoritePath) == 0)
    {
        p_game->IsFavorite = true;
    }
    else
    {
        p_game->IsFavorite = false;
    }

    return OS_SUCCESS;
}

void OS_DoAction(OS_Action_t action)
{
    OS_State_t temp = OS_CurrState;

    switch (action)
    {
        case OS_NO_ACTION:
            break;

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
