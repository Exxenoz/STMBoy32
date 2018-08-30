#include <stdlib.h>
#include <string.h>
#include "os.h"



OS_GameEntry_t OS_CurrentGame;                      // Path of the currenty/last played game.  

OS_State_t     OS_CurrState          = OS_MAINPAGE;		// Current Operatingsystem state.
OS_State_t     OS_LastState          = OS_MAINPAGE;     // Last Operatingsystem state.

int            OS_LoadedGamesCounter = 0;               // Number of successfully loaded Games.
int            OS_TotalGamesCounter  = 0;               // Number of all games detected on SDC.

bool           OS_EditBrightnessMode = false;
bool           OS_EditLanguageMode   = false;

OS_GameEntry_t OS_GameEntries[OS_MAX_NUMBER_OF_GAMES];  // Array containing all game titles & their favorite status.
OS_GameEntry_t OS_LastPlayed[OS_LAST_PLAYED_GAMES_NUM]; // Array containing last played game titles & their favorite status.

OS_Options_t   OS_Options =                             // Default Settings.
{
    .Brightness = 100,
    .AutoBoot   = false,
    .DrawScaled = true,
    .Language   = OS_ENGLISH,
};



/******************************************************************************/
/*                            OS private Functions                            */
/******************************************************************************/

// Compare Function used to sort all game entries
int CmpGameEntries(const void *a, const void *b)
{
    OS_GameEntry_t game1 = *((OS_GameEntry_t*)a);
    OS_GameEntry_t game2 = *((OS_GameEntry_t*)b);

    return CompareStrings(game1.Name, game2.Name);
}

Error_Def_t LoadOptions(void)
{
    FIL         file;  
    uint32_t    bytesRead;
    uint32_t    bytesWritten;
    Error_Def_t err_def;


    // Mount SDC if possible.
    if (!SDC_Mount())
    {
        ERR_DEF_INIT_NO_ARGUMENT(err_def, SDC_ERROR_NO_SDC);
        return err_def;
    }

    // Open the file. If it can't be opened try to create a new one and initialize it.
    if (f_open(&file, OS_OPTIONS_FILE, FA_OPEN_EXISTING | FA_READ) != FR_OK)
    {
        // Ty to create new one.
        if (f_open(&file, OS_OPTIONS_FILE, FA_CREATE_NEW | FA_WRITE) != FR_OK)
        {
            ERR_DEF_INIT(err_def, SDC_ERROR_FILE_ACCESS_FAILED, OS_OPTIONS_FILE);
            return err_def;
        }
        else
        {
            // Try to initialize new one.
            if (f_write(&file, &OS_Options, sizeof(OS_Options), &bytesWritten) != FR_OK || bytesWritten != sizeof(OS_Options))
            {
                ERR_DEF_INIT(err_def, SDC_ERROR_WRITING_FILE_FAILED, OS_OPTIONS_FILE);
                return err_def;
            }
        }
    }

    // Try to read the file.
    if (f_read(&file, &OS_Options, sizeof(OS_Options), &bytesRead) != FR_OK || bytesRead != sizeof(OS_Options))
    {
        ERR_DEF_INIT(err_def, SDC_ERROR_READING_FILE_FAILED, OS_OPTIONS_FILE);
        return err_def;
    }

    // If everything is ok close the file.
    f_close(&file);
 

    ERR_DEF_INIT_NO_ARGUMENT(err_def, OS_SUCCESS);
    return err_def;
}


/******************************************************************************/
/*                            OS public Functions                             */
/******************************************************************************/

void OS_Initialize(void)
{
    // Load initial options
    LoadOptions();
    
    // If auto boot is enabled and a cartridge is present start it immediately.
    // If there is no cartridge present try to start the last played game.
    // If there is no data about the last played game available try to load the first favorite game.
    // If no favorite game could be loaded try to load the first normal game. If this fails aswell open the mainpage.
    if (OS_Options.AutoBoot)
    {
        if (CMOD_CheckForCartridge())
        {
            OS_CurrState = OS_INGAME_FROM_CARTRIDGE;
            OS_LastState = OS_CurrState;
        }
        else
        {
            OS_LoadLastPlayed();

            if (OS_GameEntries[0].Name[0] == 0)
            {
                OS_LoadGameEntries("A", false, true);

                if (OS_GameEntries[0].Name[0] == 0)
                {
                    OS_LoadGameEntries("A", false, false);
                }
            }
            
            if (OS_GameEntries[0].Name[0] != 0)
            {
                OS_CurrentGame = OS_GameEntries[0];
                OS_CurrState   = OS_INGAME_FROM_SDC;
                OS_LastState   = OS_CurrState;
            }
        }
    }
}

Error_Def_t OS_LoadLastPlayed(void)
{   
    FIL         file;  
    uint32_t    bytesRead;
    uint32_t    bytesWritten;
    Error_Def_t err_def;

    // Clear game entries.
    memset(OS_GameEntries, 0, sizeof(OS_GameEntries));
    memset(OS_LastPlayed,  0, sizeof(OS_LastPlayed));

    // Mount SDC if possible.
    if (!SDC_Mount())
    {
        OS_LoadedGamesCounter = 0;

        ERR_DEF_INIT_NO_ARGUMENT(err_def, SDC_ERROR_NO_SDC);
        return err_def;
    }

    // Open the file. If it can't be opened try to create a new one and initialize it.
    if (f_open(&file, OS_LAST_PLAYED_FILE, FA_OPEN_EXISTING | FA_READ) != FR_OK)
    {
        // Ty to create new one.
        if (f_open(&file, OS_LAST_PLAYED_FILE, FA_CREATE_NEW | FA_WRITE) != FR_OK)
        {
            OS_LoadedGamesCounter = 0;

            ERR_DEF_INIT(err_def, SDC_ERROR_FILE_ACCESS_FAILED, OS_LAST_PLAYED_FILE);
            return err_def;
        }
        else
        {
            // Try to initialize new one.
            if (f_write(&file, OS_LastPlayed, sizeof(OS_LastPlayed), &bytesWritten) != FR_OK || bytesWritten != sizeof(OS_LastPlayed))
            {
                ERR_DEF_INIT(err_def, SDC_ERROR_WRITING_FILE_FAILED, OS_LAST_PLAYED_FILE);
                return err_def;
            }
        }
    }

    // Try to read the file.
    if (f_read(&file, OS_LastPlayed, sizeof(OS_LastPlayed), &bytesRead) != FR_OK || bytesRead != sizeof(OS_LastPlayed))
    {
        OS_LoadedGamesCounter = 0;

        ERR_DEF_INIT(err_def, SDC_ERROR_READING_FILE_FAILED, OS_LAST_PLAYED_FILE);
        return err_def;
    }

    // If everything is ok close the file and reset OS_LoadedGamesCounter.
    f_close(&file);
    OS_LoadedGamesCounter = 0;

    // Get path of each game and try to open it in order to validate it.
    // If a game is valid determine wheter it's currently marked as favorite, copy it to OS_GameEntries an increase the counter.
    for (int i = 0; i < OS_LAST_PLAYED_GAMES_NUM; i++)
    {
        char path[OS_MAX_PATH_LENGTH];
        OS_GetGamePath(&OS_LastPlayed[i], path, OS_MAX_PATH_LENGTH);
        
        if (f_open(&file, path, FA_OPEN_EXISTING | FA_READ) == FR_OK)
        {
            f_close(&file);

            OS_GameEntries[OS_LoadedGamesCounter] = OS_LastPlayed[i];
            OS_Set_IsFavorite(&OS_GameEntries[OS_LoadedGamesCounter]);

            OS_LoadedGamesCounter++;
        }
    }
    OS_TotalGamesCounter = OS_LoadedGamesCounter;


    ERR_DEF_INIT_NO_ARGUMENT(err_def, OS_SUCCESS);
    return err_def;
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
    memset(OS_GameEntries, 0, sizeof(OS_GameEntries));

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
            if (previous && CompareStrings(startingName, fileInfo.fname) < 0) continue;
            if (!previous && CompareStrings(startingName, fileInfo.fname) > 0) continue;

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
                    currentDifferenze = CompareStrings(OS_GameEntries[i].Name, fileInfo.fname);
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
                    qsort(OS_GameEntries, OS_LoadedGamesCounter, sizeof(OS_GameEntry_t), CmpGameEntries);

                    ERR_DEF_INIT(err_def, SDC_ERROR_DIR_ACCESS_FAILED, OS_FAVORITE_DIRECTORY);
                    return err_def;
                }
            }
        }

    } while (!finishedLoading);

    // Close directory and sort the list alphabetically (case insensitive)
    f_closedir(&directory);
    qsort(OS_GameEntries, OS_LoadedGamesCounter, sizeof(OS_GameEntry_t), CmpGameEntries);

    ERR_DEF_INIT_NO_ARGUMENT(err_def, OS_SUCCESS);
    return err_def;
}

Error_Def_t OS_RemoveGameEntry(int currGameEntryIndex)
{
    Error_Def_t err_def;

    if (currGameEntryIndex >= OS_LoadedGamesCounter)
    {
        ERR_DEF_INIT(err_def, OS_ERROR_INVALID_INDEX, (char*)(&currGameEntryIndex));
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

Error_Def_t OS_UpdateOptions(void)
{
    FIL         file;  
    uint32_t    bytesWritten;
    Error_Def_t err_def;


    // Mount SDC if possible.
    if (!SDC_Mount())
    {
        ERR_DEF_INIT_NO_ARGUMENT(err_def, SDC_ERROR_NO_SDC);
        return err_def;
    }

    // Open the file. If it can't be opened try to create a new one.
    if (f_open(&file, OS_OPTIONS_FILE, FA_OPEN_EXISTING | FA_WRITE) != FR_OK)
    {
        if (f_open(&file, OS_OPTIONS_FILE, FA_CREATE_NEW | FA_WRITE) != FR_OK)
        {
            ERR_DEF_INIT(err_def, SDC_ERROR_FILE_ACCESS_FAILED, OS_OPTIONS_FILE);
            return err_def;
        }
    }

    // Write OS_Options to the options file.
    if (f_write(&file, &OS_Options, sizeof(OS_Options), &bytesWritten) != FR_OK || bytesWritten != sizeof(OS_Options))
    {
        ERR_DEF_INIT(err_def, SDC_ERROR_WRITING_FILE_FAILED, OS_OPTIONS_FILE);
        return err_def;
    }

    // If everything is ok close the file.
    f_close(&file);
 

    ERR_DEF_INIT_NO_ARGUMENT(err_def, OS_SUCCESS);
    return err_def;
}

Error_Def_t OS_UpdateLastPlayed(void)
{    
    Error_Def_t err_def;
    uint32_t    bytesWritten;
    uint32_t    bytesRead;
    FIL         file;

    // Clear game entries.
    memset(OS_LastPlayed,  0, sizeof(OS_LastPlayed));

    // If no SDC can be mounted return false
    if (!SDC_Mount())
    {
        ERR_DEF_INIT_NO_ARGUMENT(err_def, SDC_ERROR_NO_SDC);
        return err_def;
    }

    // Open the file. If it can't be opened try to create a new one and initialize it with the current game.
    if (f_open(&file, OS_LAST_PLAYED_FILE, FA_OPEN_EXISTING | FA_READ | FA_WRITE) != FR_OK)
    {
        // Ty to create new one.
        if (f_open(&file, OS_LAST_PLAYED_FILE, FA_CREATE_NEW | FA_WRITE) != FR_OK)
        {
            OS_LoadedGamesCounter = 0;

            ERR_DEF_INIT(err_def, SDC_ERROR_FILE_ACCESS_FAILED, OS_LAST_PLAYED_FILE);
            return err_def;
        }
        else
        {
            // Set the first OS_LastPlayed entry to the current game if OS_LastPlayed wasn't initialized yet (Name wouldn't start with 0).
            if (OS_LastPlayed[0].Name[0] != 0)
            {
                OS_LastPlayed[0] = OS_CurrentGame;
            }

            // Try to initialize the new file.
            if (f_write(&file, OS_LastPlayed, sizeof(OS_LastPlayed), &bytesWritten) != FR_OK || bytesWritten != sizeof(OS_LastPlayed))
            {
                ERR_DEF_INIT(err_def, SDC_ERROR_WRITING_FILE_FAILED, OS_LAST_PLAYED_FILE);
                return err_def;
            }
        }
    }
    else // Update the lastPlayed file
    {
        // Try to read the lastPlayed entries.
        if (f_read(&file, OS_LastPlayed, sizeof(OS_LastPlayed), &bytesRead) != FR_OK)
        {
            ERR_DEF_INIT(err_def, SDC_ERROR_READING_FILE_FAILED, OS_LAST_PLAYED_FILE);
            return err_def;
        }

        // If current game is already stored as one of the lastPlayed get it's position otherwise get the last position.
        int removal_start_index = OS_LAST_PLAYED_GAMES_NUM - 1;
        for (int i = 0; i < OS_LAST_PLAYED_GAMES_NUM; i++)
        {
            if (CompareStrings(OS_LastPlayed[i].Name, OS_CurrentGame.Name) == 0)
            {
                removal_start_index = i;
                break;
            }
        }

        // If the first last played game isn't the current game,
        // move all previous game entries 1 position down starting at the determined position (last gets deleted) and store current game on the first position.
        for (int i = removal_start_index; i > 0; i--)
        {
            OS_LastPlayed[i] = OS_LastPlayed[i - 1];
        }
        OS_LastPlayed[0] = OS_CurrentGame;

        // Store the last played games.
        f_lseek(&file, 0);
        if (f_write(&file, OS_LastPlayed, sizeof(OS_LastPlayed), &bytesWritten) != FR_OK || bytesWritten != sizeof(OS_LastPlayed))
        {
            ERR_DEF_INIT(err_def, SDC_ERROR_WRITING_FILE_FAILED, OS_LAST_PLAYED_FILE);
            return err_def;
        }
    }
    
    // Close the file.
    f_close(&file);


    ERR_DEF_INIT_NO_ARGUMENT(err_def, OS_SUCCESS);
    return err_def;
}

Error_Def_t OS_GetGamePath(OS_GameEntry_t *p_game, char *path, int pathLength)
{
    FIL         file;
    FRESULT     test;
    Error_Def_t err_def;

    // Check if the array in which the path is to be stored is long enough.
    if (pathLength < OS_MAX_PATH_LENGTH)
    {
        ERR_DEF_INIT_NO_ARGUMENT(err_def, OS_ERROR_INVALID_PATH_LENGTH);
        return err_def;
    }

    // Check if the game is located inside the favorite folder.
    CopyString(path, OS_FAVORITE_PATH, pathLength);
    AppendString(path, p_game->Name, pathLength);
    test = f_open(&file, path, FA_OPEN_EXISTING);
    if (test == FR_OK)
    {
        f_close(&file);

        ERR_DEF_INIT_NO_ARGUMENT(err_def, OS_SUCCESS);
        return err_def;
    }

    // If the game isn't located inside the favorite folder check if its located in the game folder.
    CopyString(path, OS_GAME_PATH, pathLength);
    AppendString(path, p_game->Name, pathLength);
    test = f_open(&file, path, FA_OPEN_EXISTING);
    if (test == FR_OK)
    {
        f_close(&file);

        ERR_DEF_INIT_NO_ARGUMENT(err_def, OS_SUCCESS);
        return err_def;
    }

    // If it's located in neither it doesn't exist.
    memset(path, 0, pathLength);
    ERR_DEF_INIT(err_def, OS_ERROR_GAME_NOT_FOUND, p_game->Name);
    return err_def;
}

Error_Def_t OS_GetSaveGamePath(OS_GameEntry_t *p_game, char *path, int pathLength)
{
    Error_Def_t err_def;

    // Check if the array in which the path is to be stored is long enough.
    if (pathLength < OS_MAX_PATH_LENGTH)
    {
        ERR_DEF_INIT_NO_ARGUMENT(err_def, OS_ERROR_INVALID_PATH_LENGTH);
        return err_def;
    }

    // Get path.
    char name[OS_MAX_GAME_TITLE_LENGTH];
    CopyStringWithoutSuffix(name, p_game->Name,OS_MAX_GAME_TITLE_LENGTH);
    
    CopyString(path, OS_SAVEGAME_PATH, pathLength);
    AppendString(path, name, pathLength);
    AppendString(path, OS_SAVEGAME_SUFFIX, pathLength);


    ERR_DEF_INIT_NO_ARGUMENT(err_def, OS_SUCCESS);
    return err_def;
}

Error_Def_t OS_Set_IsFavorite(OS_GameEntry_t *p_game)
{
    Error_Def_t err_def;
    char        path[OS_MAX_PATH_LENGTH];
    char        favoritePath[OS_MAX_PATH_LENGTH];

    err_def = OS_GetGamePath(p_game, path, OS_MAX_PATH_LENGTH);
    ERROR_CHECK(err_def);

    CopyString(favoritePath, OS_FAVORITE_PATH, OS_MAX_PATH_LENGTH);
    AppendString(favoritePath, p_game->Name, OS_MAX_PATH_LENGTH);

    if (CompareStrings(path, favoritePath) == 0)
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
            OS_UpdateLastPlayed();
            break;
        
        case OS_TOGGLE_OPTION_DIRECT_BOOT:
            OS_Options.AutoBoot = (bool)!OS_Options.AutoBoot;
            OS_UpdateOptions();
            break;
        
        case OS_TOGGLE_OPTION_SCALING:
            OS_Options.DrawScaled = (bool)!OS_Options.DrawScaled;
            OS_UpdateOptions();
            break;
        
        case OS_EDIT_BRIGHTNESS:
            OS_EditBrightnessMode = (bool)!OS_EditBrightnessMode;
            break;
        
        case OS_EDIT_LANGUAGE:
            OS_EditLanguageMode = (bool)!OS_EditLanguageMode;
            break;
        
        case OS_SAVE_CARTRIDGE:
            CMOD_SaveCartridge(true);
            break;
    }
}
