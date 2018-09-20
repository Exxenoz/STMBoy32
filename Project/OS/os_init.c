#include "os.h"
#include "os_init.h"
#include "os_config.h"


TIM_HandleTypeDef OS_TimerHandle =
{ 
    .Instance = OS_TIM
};



void OS_Initialize_Timer(void)
{
    OS_TimerHandle.Init.Prescaler         = 49999;                   // Tim5 runs with 100Mhz(?) -> scale to 2khz.
    OS_TimerHandle.Init.CounterMode       = TIM_COUNTERMODE_UP;
    OS_TimerHandle.Init.Period            = 1999;                    // Count 'til 2000(-1) -> 1 Interrupt / s.
    OS_TimerHandle.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    OS_TimerHandle.Init.RepetitionCounter = 0;
    HAL_TIM_Base_Init(&OS_TimerHandle);

    HAL_NVIC_SetPriority(OS_TIM_NVIC_CHANNEL, INTERRUPT_PRIORITY_2, INTERRUPT_PRIORITY_2);
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

    OS_Initialize_Timer();
}
