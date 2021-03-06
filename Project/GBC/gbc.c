#include "gbc.h"
#include "gbc_cpu.h"
#include "gbc_gpu.h"
#include "gbc_mmu.h"
#include "gbc_apu.h"
#include "cmod_access.h"
#include "sdc.h"
#include "os.h"

GBC_LoadState_t GBC_LoadState = GBC_LOAD_STATE_NONE; // Global GBC load state

GBC_LoadResult_t GBC_LoadFromCartridge(void)
{
    GBC_Unload();

    CMOD_TurnON();
    CMOD_ResetCartridge();

    if (!CMOD_CheckForCartridge())
    {
        return GBC_LOAD_RESULT_NO_CARTRIDGE;
    }

    if (!GBC_MMU_LoadFromCartridge())
    {
        return GBC_LOAD_RESULT_NO_VALID_CARTRIDGE;
    }

    GBC_CPU_Initialize();
    GBC_GPU_Initialize();
    GBC_APU_Initialize();

    GBC_LoadState = GBC_LOAD_STATE_CARTRIDGE;
    CMOD_TurnOFF();

    return GBC_LOAD_RESULT_OK;
}

GBC_LoadResult_t GBC_LoadFromSDC(char* fileName)
{
    // ToDo: Implement UI Error Notification
    GBC_Unload();

    if (!SDC_Mount())
    {
        return GBC_LOAD_RESULT_NO_SDC;
    }

    if (!GBC_MMU_LoadFromSDC(fileName))
    {
        return GBC_LOAD_RESULT_NO_ROM_FILE;
    }

    GBC_CPU_Initialize();
    GBC_GPU_Initialize();
    GBC_APU_Initialize();

    GBC_LoadState = GBC_LOAD_STATE_SDC;

    return GBC_LOAD_RESULT_OK;
}

void GBC_Unload(void)
{
    GBC_MMU_Unload();

    GBC_LoadState = GBC_LOAD_STATE_NONE;
}

void GBC_SDC_SaveCurrentERAM(void)
{
    FIL      file;
    char     path[OS_MAX_PATH_LENGTH];
    uint32_t bytesWritten;
    
    OS_GetSaveGamePath(&OS_CurrentGame, path, OS_MAX_PATH_LENGTH);
    
    f_open(&file, path, FA_OPEN_ALWAYS | FA_WRITE);
    f_write(&file, GBC_MMU_Memory.ERAMBank0, 4 * 8192, &bytesWritten);
    f_close(&file);
}

GBC_LoadResult_t GBC_SDC_LoadERAM(char *path)
{
    FIL      file;
    uint32_t bytesRead;

    f_open(&file, path, FA_OPEN_EXISTING | FA_READ);
    if (f_read(&file, GBC_MMU_Memory.ERAMBank0, 4 * 8192, &bytesRead) != FR_OK)
    {
        f_close(&file);
        return GBC_LOAD_RESULT_NO_VALID_SAV_FILE;
    }

    f_close(&file);
    return GBC_LOAD_RESULT_OK;
}

bool GBC_IsLoaded(void)
{
    return GBC_LoadState != GBC_LOAD_STATE_NONE ? true : false;
}

bool GBC_IsLoadedFromCartridge(void)
{
    return GBC_LoadState == GBC_LOAD_STATE_CARTRIDGE ? true : false;
}

bool GBC_IsLoadedFromSDC(void)
{
    return GBC_LoadState == GBC_LOAD_STATE_SDC ? true : false;
}

void GBC_Update(void)
{
    uint32_t cacheForStepTicks = 0;

    // Update until VBlank occurs
    do
    {
        GBC_CPU_Step();

        // Double Speed Mode
        if (GBC_CPU_SpeedModifier)
        {
            // Divide CPU step ticks by 2
            GBC_CPU_StepTicks >>= 1;
            // Cache CPU step ticks from first CPU step
            cacheForStepTicks = GBC_CPU_StepTicks;
            // Process next CPU step; GBC_CPU_StepTicks will be overwritten
            GBC_CPU_Step();
            // Divide CPU step ticks by 2
            GBC_CPU_StepTicks >>= 1;
            // Add ticks from first CPU step
            GBC_CPU_StepTicks += cacheForStepTicks;
        }

        GBC_APU_Step();
    }
    while (!GBC_GPU_Step());
}
