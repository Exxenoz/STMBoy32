#include <stdlib.h>
#include <string.h>
#include "os.h"


OS_Options_t   OS_InitOptions;                          // Inititial Options

OS_GameEntry_t OS_CurrentGame;                          // Path of the currenty/last played game      

OS_State_t     OS_CurrState          = OS_MAINPAGE;		// Current Operatingsystem state
OS_State_t     OS_LastState          = OS_MAINPAGE;    // Last Operatingsystem state

int            OS_LoadedGamesCounter = 0;               // Number of successfully loaded Games
int            OS_TotalGamesCounter  = 0;               // Number of all games detected on SDC

OS_GameEntry_t OS_GameEntries[OS_MAX_NUMBER_OF_GAMES];  // Array containing all game titles & their favorite status



void OS_Initialize(void)
{
    // Load initial options
    OS_LoadOptions();
}

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
Error_Def_t OS_LoadGameEntries(char* startingName, bool previous, bool onlyFavorites)
{
    DIR         directory;
    char       *directoryPath;
    bool        isFavorite;
    FILINFO     fileInfo;
    Error_Def_t err_def;

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
        ERR_DEF_INIT_NO_ARGUMENT(err_def, SDC_ERROR_NO_SDC);
        return err_def;
    }

    if (f_opendir(&directory, directoryPath) != FR_OK)
    {
        ERR_DEF_INIT(err_def, SDC_ERROR_DIR_ACCESS_FAILED, directoryPath);
        return err_def;
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
                isFavorite    = true;

                // Close the game-directory
                f_closedir(&directory);

                // If opening the favorite directory fails sort what was already stored and exit
                if (f_opendir(&directory, OS_FAVORITE_DIRECTORY) != FR_OK)
                {
                    qsort(OS_GameEntries, OS_LoadedGamesCounter, sizeof(OS_GameEntry_t), OS_CmpGameEntries);

                    ERR_DEF_INIT(err_def, SDC_ERROR_DIR_ACCESS_FAILED, OS_FAVORITE_DIRECTORY);
                    return err_def;
                }
            }
        }

    } while (!finishedLoading);

    // Close directory and sort the list alphabetically (case insensitive)
    f_closedir(&directory);
    qsort(OS_GameEntries, OS_LoadedGamesCounter, sizeof(OS_GameEntry_t), OS_CmpGameEntries);

    ERR_DEF_INIT_NO_ARGUMENT(err_def, OS_SUCCESS);
    return err_def;
}

Error_Def_t OS_InvertFavoriteStatus(OS_GameEntry_t *p_game)
{
    char        oldPath[OS_MAX_PATH_LENGTH];
    char        newPath[OS_MAX_PATH_LENGTH];
    Error_Def_t err_def;

    // Get old path
    err_def = OS_GetGamePath(p_game, oldPath, OS_MAX_PATH_LENGTH);
    ERROR_CHECK(err_def);

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

    ERR_DEF_INIT_NO_ARGUMENT(err_def, OS_SUCCESS);
    return err_def;
}

int ConvertCharArrayToGameEntries(const char *buffer, int bufferSize, OS_GameEntry_t *GameEntries, int entries)
{
    int currGameIndex = 0;

    for (int i = 0, j = 0; i < bufferSize && buffer[i] != '-' && currGameIndex < entries; i++, j++)
    {        
        if (buffer[i] != '\r')
        {
            OS_GameEntries[currGameIndex].Name[j] = buffer[i];
        }
        else
        {
            OS_GameEntries[currGameIndex].Name[j] = '\0';

            ERROR_CHECK(OS_IsFavorite(&(OS_GameEntries[currGameIndex])));

            currGameIndex++;
            j = -1;
            i++;             // Skip the '\n'
        }
    }

    return currGameIndex;
}

Error_Def_t OS_LoadLastPlayed(void)
{
    //ToDo:
    //Implement function to convert lastPlayedGames to a char[]
    //Use this new approach for OS_-Load/Update-LastPlayed
    
    // The game titles are stored without null termination followed by '\r''\n', end of file is indicated by '-'
    FIL         file;  
    int         bufferSize = OS_LAST_PLAYED_GAMES_NUM * (OS_MAX_GAME_TITLE_LENGTH + 2) + 1;
    char        buffer[bufferSize];
    uint32_t    bytesRead;
    Error_Def_t err_def;

    // Clear OS_GameEntries
    memset(OS_GameEntries, 0, OS_MAX_NUMBER_OF_GAMES * sizeof(OS_GameEntry_t));

    // If no SDC can be mounted, file can't be opened/created or (fully) read set gameCounter to 0
    if (!SDC_Mount())
    {
        OS_LoadedGamesCounter = 0;

        ERR_DEF_INIT_NO_ARGUMENT(err_def, SDC_ERROR_NO_SDC);
        return err_def;
    }

    if (f_open(&file, OS_LAST_PLAYED_FILE, FA_OPEN_EXISTING | FA_READ) != FR_OK)
    {
        OS_LoadedGamesCounter = 0;

        ERR_DEF_INIT(err_def, SDC_ERROR_FILE_ACCESS_FAILED, OS_LAST_PLAYED_FILE);
        return err_def;
    }

    if (f_read(&file, buffer, bufferSize, &bytesRead) != FR_OK || bytesRead != bufferSize)
    {
        OS_LoadedGamesCounter = 0;

        ERR_DEF_INIT(err_def, SDC_ERROR_READING_FILE_FAILED, OS_LAST_PLAYED_FILE);
        return err_def;
    }

    // If everything is ok close the file, load the games and set gameCounter accordingly
    f_close(&file);

    OS_LoadedGamesCounter = ConvertCharArrayToGameEntries(buffer, bufferSize, OS_GameEntries, OS_MAX_NUMBER_OF_GAMES);
    OS_TotalGamesCounter  = OS_LoadedGamesCounter;

    ERR_DEF_INIT_NO_ARGUMENT(err_def, OS_SUCCESS);
    return err_def;
}

Error_Def_t OS_UpdateLastPlayed(void)
{    
    Error_Def_t err_def;
    uint32_t    bytesWritten;
    uint32_t    bytesRead;
    FIL         file;

    // If no SDC can be mounted return false
    if (!SDC_Mount())
    {
        ERR_DEF_INIT_NO_ARGUMENT(err_def, SDC_ERROR_NO_SDC);
        return err_def;
    }

    // If the file doesn't exist try to create a new one, if it does update it
    if (f_open(&file, OS_LAST_PLAYED_FILE, FA_OPEN_EXISTING | FA_READ | FA_WRITE) == FR_NO_FILE)
    {
        // If creating a new file or writing to it fails return false
        if (f_open(&file, OS_LAST_PLAYED_FILE, FA_CREATE_NEW | FA_WRITE) != FR_OK)
        {
            ERR_DEF_INIT(err_def, SDC_ERROR_FILE_ACCESS_FAILED, OS_LAST_PLAYED_FILE);
            return err_def;
        }

        // Initialize buffer with the first line (containing first game title) and 0s for the rest
        int bufferSize = CountChars(OS_CurrentGame.Name) + 2;
        char buffer[bufferSize];

        CopyString(buffer, OS_CurrentGame.Name, bufferSize);
        AppendChars(buffer, "\r\n", bufferSize, 2);

        if (f_write(&file, buffer, bufferSize, &bytesWritten) != FR_OK || bytesWritten != bufferSize)
        {
            ERR_DEF_INIT(err_def, SDC_ERROR_READING_FILE_FAILED, OS_LAST_PLAYED_FILE);
            return err_def;
        }
        
        f_close(&file);

        ERR_DEF_INIT_NO_ARGUMENT(err_def, OS_SUCCESS);
        return err_def;
    }
    else // Update the lastPlayed file
    {
        int            bufferSize = OS_LAST_PLAYED_GAMES_NUM * (OS_MAX_GAME_TITLE_LENGTH + 2) + 1;
        char           buffer[bufferSize];
        OS_GameEntry_t lastPlayedGames[OS_LAST_PLAYED_GAMES_NUM];

        // Read the lastPlayed entries to buffer
        if (f_read(&file, buffer, bufferSize, &bytesRead) != FR_OK)
        {
            ERR_DEF_INIT(err_def, SDC_ERROR_READING_FILE_FAILED, OS_LAST_PLAYED_FILE);
            return err_def;
        }

        // If current game is already stored as one of the lastPlayed get it's position otherwise the last one
        // Then move all entries 1 position down (last gets deleted) and store current game on the first position
        // If current game already was in the list start there (-> basically just moves it to the first position)
        int lastPlayedNum = ConvertCharArrayToGameEntries(buffer, bufferSize, lastPlayedGames, OS_LAST_PLAYED_GAMES_NUM);

        for (int i = 0; i <= lastPlayedNum; i++)
        {
            if (strcmp(lastPlayedGames[i].Name, OS_CurrentGame.Name) == 0 || i == lastPlayedNum)
            {
                for (; i > 0; i--)
                {
                    lastPlayedGames[i] = lastPlayedGames[i-1];
                }
                
                lastPlayedGames[0] = OS_CurrentGame;
                break;
            }
        }    

        // Calculate correct (exact) bufferSize, convert lastPlayedGames to char[] and write it back into the file
        bufferSize = CountChars(lastPlayedGames[0].Name) + 3;
        CopyString(buffer, lastPlayedGames[0].Name, bufferSize);
        AppendString(buffer, "\r\n", bufferSize);

        for (int i = 1; i < lastPlayedNum; i++)
        {
            bufferSize += CountChars(lastPlayedGames[i].Name) + 2;
            AppendString(buffer, lastPlayedGames[i].Name, bufferSize);
            AppendString(buffer, "\r\n", bufferSize);
        }

        bufferSize += CountChars("--End of File--");
        AppendChars(buffer, "--End of File--", bufferSize, 15);

        f_lseek(&file, 0);
        if (f_write(&file, buffer, bufferSize, &bytesWritten) != FR_OK || bytesWritten != bufferSize)
        {
            ERR_DEF_INIT(err_def, SDC_ERROR_WRITING_FILE_FAILED, OS_LAST_PLAYED_FILE);
            return err_def;
        }
    }
    f_close(&file);

    ERR_DEF_INIT_NO_ARGUMENT(err_def, OS_SUCCESS);
    return err_def;
}

Error_Def_t OS_GetGameEntry(char *name, OS_GameEntry_t **gameEntry)
{
    Error_Def_t err_def;

    for (int i = 0; i < OS_LoadedGamesCounter; i++)
    {
        if (CmpStrings(OS_GameEntries[i].Name, name) == 0)
        {
            *gameEntry = &(OS_GameEntries[i]);

            ERR_DEF_INIT_NO_ARGUMENT(err_def, OS_SUCCESS);
            return err_def;
        }
    }

    ERR_DEF_INIT(err_def, OS_ERROR_GAME_NOT_FOUND, name);
    return err_def;
}

Error_Def_t OS_RemoveGameEntry(int currGameEntryIndex)
{
    Error_Def_t err_def;

    if (currGameEntryIndex >= OS_LoadedGamesCounter)
    {
        ERR_DEF_INIT(err_def, OS_ERROR_INVALID_INDEX, (char*)(&currGameEntryIndex)); // ?
        return err_def;
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

    ERR_DEF_INIT_NO_ARGUMENT(err_def, OS_SUCCESS);
    return err_def;
}

Error_Def_t OS_GetGamePath(OS_GameEntry_t *p_game, char *path, int pathLength)
{
    FIL         file;
    FRESULT     test;
    Error_Def_t err_def;

    // Check if the array in which the path is to be stored is long enough
    if (pathLength < OS_MAX_PATH_LENGTH)
    {
        ERR_DEF_INIT_NO_ARGUMENT(err_def, OS_ERROR_INVALID_PATH_LENGTH);
        return err_def;
    }

    // Check if the game is located inside the favorite folder
    CopyString(path, OS_FAVORITE_PATH, pathLength);
    AppendString(path, p_game->Name, pathLength);
    test = f_open(&file, path, FA_OPEN_EXISTING);
    if (test == FR_OK)
    {
        f_close(&file);

        ERR_DEF_INIT_NO_ARGUMENT(err_def, OS_SUCCESS);
        return err_def;
    }

    // If the game isn't located inside the favorite folder check if its located in the game folder
    CopyString(path, OS_GAME_PATH, pathLength);
    AppendString(path, p_game->Name, pathLength);
    test = f_open(&file, path, FA_OPEN_EXISTING);
    if (test == FR_OK)
    {
        f_close(&file);

        ERR_DEF_INIT_NO_ARGUMENT(err_def, OS_SUCCESS);
        return err_def;
    }

    // If it's located in neither it doesn't exist
    memset(path, 0, pathLength);
    ERR_DEF_INIT(err_def, OS_ERROR_GAME_NOT_FOUND, p_game->Name);
    return err_def;
}

Error_Def_t OS_IsFavorite(OS_GameEntry_t *p_game)
{
    Error_Def_t err_def;
    char        path[OS_MAX_PATH_LENGTH];
    char        favoritePath[OS_MAX_PATH_LENGTH];

    err_def = OS_GetGamePath(p_game, path, OS_MAX_PATH_LENGTH);
    ERROR_CHECK(err_def);

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

    ERR_DEF_INIT_NO_ARGUMENT(err_def, OS_SUCCESS);
    return err_def;
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

        case OS_SWITCH_TO_STATE_MAINPAGE:
            OS_LastState = OS_CurrState;
            OS_CurrState = OS_MAINPAGE;
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
