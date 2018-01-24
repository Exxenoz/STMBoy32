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
int OS_LoadedGamesCounter = 0;                         // Number of successfully loaded Games
int OS_TotalGamesCounter = 0;                          // Number of all games detected on SDC


// Compare two gametitles if title b is alphabetically 'higher' the return value is < 0
int OS_CmpGameTitles(const void *a, const void *b)
{
    char *title1 = (char*)a;
    char *title2 = (char*)b;

    // Get the first different letters and compare them (case insensitive)
    int i = -1;
    do
    {
        i++;
        if (title1[i] >= 97 && title1[i] <= 122)
        {
            title1[i] -= 32;
        }
        if (title2[i] >= 97 && title2[i] <= 122)
        {
            title2[i] -= 32;
        }

    } while (title1[i] == title2[i] && title1[i] != '\0');

    return (title1[i] - title2[i]);
}

// Compare Function used to sort all game entries
int OS_CmpGameEntries(const void *a, const void *b)
{
    OS_GameEntry_t game1 = *((OS_GameEntry_t*)a);
    OS_GameEntry_t game2 = *((OS_GameEntry_t*)b);

    return OS_CmpGameTitles(game1.Name, game2.Name);
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
void OS_LoadGameEntries(char* startingName, bool previous, bool onlyFavorites)
{
    DIR     directory;
    char    *directoryPath;
    bool    isFavorite;
    FILINFO fileInfo;

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
    if (!SDC_Mount() || f_opendir(&directory, directoryPath) != FR_OK) return;

    // Get the alphabetically 'lowest' OS_MAX_NUMBER_OF_GAMES titles 'higher' than startingName from the directory
    bool finishedLoading = false;
    do
    {
        if (f_readdir(&directory, &fileInfo) == FR_OK && fileInfo.fname[0] != NULL && fileInfo.fattrib != AM_DIR)
        {
            // If the read gametitle is not 'higher'/'lower' than startingName continue
            if (previous && OS_CmpGameTitles(startingName, fileInfo.fname) < 0) continue;
            if (!previous && OS_CmpGameTitles(startingName, fileInfo.fname) > 0) continue;

            // If OS_GameEntries is not fully populated add the name of the game unconditionally
            if (OS_LoadedGamesCounter < OS_MAX_NUMBER_OF_GAMES)
            {
                copyString(OS_GameEntries[OS_LoadedGamesCounter].Name, fileInfo.fname, OS_MAX_GAME_TITLE_LENGTH + 1);
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
                    currentDifferenze = OS_CmpGameTitles(OS_GameEntries[i].Name, fileInfo.fname);
                    if (currentDifferenze > biggestDifferenze)
                    {
                        biggestDifferenze = currentDifferenze;
                        highestTitleIndex  = i;
                    }
                }

                // If it is, replace the alphabetically 'highest' stored title with the currently read title
                if (highestTitleIndex != -1)
                {
                    copyString(OS_GameEntries[highestTitleIndex].Name, fileInfo.fname, OS_MAX_GAME_TITLE_LENGTH + 1);
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
                    return;
                }
            }
        }

    } while (!finishedLoading);

    // Close directory and sort the list alphabetically (case insensitive)
    f_closedir(&directory);
    qsort(OS_GameEntries, OS_LoadedGamesCounter, sizeof(OS_GameEntry_t), OS_CmpGameEntries);
}

void OS_UpdateFavorite(OS_GameEntry_t *p_game)
{
    // Define path array with maximal needed size
    int  pathLength = sizeof(OS_FAVORITE_DIRECTORY) + OS_MAX_GAME_TITLE_LENGTH + 1;
    char oldPath[pathLength];
    char newPath[pathLength];

    // Get old path, change the favorite attribute and then get the new path
    OS_GetGamePath(p_game, oldPath, pathLength);
    p_game->IsFavorite = p_game->IsFavorite ? false : true;
    OS_GetGamePath(p_game, newPath, pathLength);

    // Move the game into the new directory
    f_rename(oldPath, newPath);
}

void OS_LoadLastPlayed(void)
{
    OS_Test test;
    // The game titles are stored padded by 0bytes (OS_MAX_GAME_TITLE_LENGTH + 1 bytes) followed by '\r''\n'
    FIL      file;  
    int      bufferSize = OS_LAST_PLAYED_GAMES_NUM * (OS_MAX_GAME_TITLE_LENGTH + 3);
    char     buffer[bufferSize];
    uint32_t bytesRead;

    // If no SDC can be mounted, file can't be opened/created or (fully) read set gameCounter to 0
    if (!SDC_Mount() || f_open(&file, OS_LAST_PLAYED_FILE, FA_OPEN_EXISTING | FA_READ) != FR_OK ||
        f_read(&file, test.buffer, bufferSize, &bytesRead) != FR_OK ||
        bytesRead != bufferSize)
    {
        OS_LoadedGamesCounter = 0;
        return;
    }
    f_close(&file);

    // If everything is ok, load the games and set gameCounter accordingly
    int  currPosition = 0;
    int  i = 0;

    for (; i < OS_LAST_PLAYED_GAMES_NUM; i++, currPosition += (OS_MAX_GAME_TITLE_LENGTH + 3))
    {        
        if (test.buffer[currPosition] == '\0') break;

        copyChars(OS_GameEntries[i].Name, &(test.buffer[currPosition]), OS_MAX_GAME_TITLE_LENGTH + 1);
        OS_GameEntries[i].IsFavorite = OS_IsFavorite(&(test.buffer[currPosition]));
    }

    OS_LoadedGamesCounter = i;
    OS_TotalGamesCounter  = i;
}

bool OS_UpdateLastPlayed(void)
{
    FIL file;

    // The game titles are stored padded by 0bytes (OS_MAX_GAME_TITLE_LENGTH + 1 bytes) followed by '\r''\n'
    int      bufferSize = OS_LAST_PLAYED_GAMES_NUM * (OS_MAX_GAME_TITLE_LENGTH + 3);
    char     buffer[bufferSize];
    uint32_t bytesWritten;
    uint32_t bytesRead;

    // If no SDC can be mounted return false
    if (!SDC_Mount()) return false;

    // If the file doesn't exist try to create a new one, if it does update it
    if (f_open(&file, OS_LAST_PLAYED_FILE, FA_OPEN_EXISTING | FA_READ | FA_WRITE) != FR_OK)
    {
        // Initialize buffer with the first line (containing first game title) and 0s for the rest
        memset(buffer, '\0', bufferSize);
        copyString(buffer, OS_CurrentGame.Name, OS_MAX_GAME_TITLE_LENGTH + 1);
        buffer[OS_MAX_GAME_TITLE_LENGTH + 1] = '\r';
        buffer[OS_MAX_GAME_TITLE_LENGTH + 2] = '\n';

        // If creating a new file or writing to it fails return false
        if (f_open(&file, OS_LAST_PLAYED_FILE, FA_CREATE_NEW | FA_WRITE) != FR_OK ||
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
        int i = 0;

        // Read the lastPlayed entries to buffer
        if (f_read(&file, buffer, bufferSize, &bytesRead) != FR_OK) return false;
        
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

            copyChars(&(buffer[currPosition]), &(buffer[lastPosition]), OS_MAX_GAME_TITLE_LENGTH + 3);
        }

        // Get the string representing the current game
        char paddedName[OS_MAX_GAME_TITLE_LENGTH + 3];
        memset(paddedName, 0, sizeof(paddedName));

        copyString(paddedName, OS_CurrentGame.Name, OS_MAX_GAME_TITLE_LENGTH + 1);
        paddedName[OS_MAX_GAME_TITLE_LENGTH + 1] = '\r';
        paddedName[OS_MAX_GAME_TITLE_LENGTH + 2] = '\n';
        
        copyChars(buffer, paddedName, OS_MAX_GAME_TITLE_LENGTH + 3);

        // Store current game on position 1 of last played games and write buffer back into OS_LAST_PLAYED_FILE
        f_lseek(&file, 0);
        if (f_write(&file, buffer, bufferSize, &bytesWritten) != FR_OK || bytesWritten != bufferSize) return false;
    }

    f_close(&file);
    return true;
}

OS_GameEntry_t OS_GetGameEntry(char *name)
{
    int c;

    for (int i = 0; i < OS_LoadedGamesCounter; i++)
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

void OS_RemoveGameEntry(int currGameEntryIndex)
{
    for (int i = currGameEntryIndex; i < OS_LoadedGamesCounter; i++)
    {
        if (i == OS_LoadedGamesCounter - 1)
        {
            copyString(OS_GameEntries[i].Name, "", OS_MAX_GAME_TITLE_LENGTH + 1);
        }
        else
        {
            copyString(OS_GameEntries[i].Name, OS_GameEntries[i + 1].Name, OS_MAX_GAME_TITLE_LENGTH + 1);
            OS_GameEntries[i].IsFavorite = OS_GameEntries[i + 1].IsFavorite;
        }
    }

    OS_LoadedGamesCounter--;
    OS_TotalGamesCounter--;
}

void OS_GetGamePath(OS_GameEntry_t *p_game, char *path, int pathLength)
{
    if (p_game->IsFavorite)
    {
        copyString(path, OS_FAVORITE_PATH, pathLength);
        appendString(path, p_game->Name, pathLength);
    }
    else
    {
        copyString(path, OS_GAME_PATH, pathLength);
        appendString(path, p_game->Name, pathLength);
    }
}

bool OS_IsFavorite(char *name)
{
    FIL  file;
    int  pathLength = sizeof(OS_FAVORITE_PATH) + OS_MAX_GAME_TITLE_LENGTH + 1;
    char path[pathLength];

    // If the game is located inside the favorite folder it's a favorite
    copyString(path, OS_FAVORITE_PATH, pathLength);
    appendString(path, name, pathLength);
    FRESULT test = f_open(&file, path, FA_OPEN_EXISTING);
    if (test == FR_OK)
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
