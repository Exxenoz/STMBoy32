#include "gbc.h"
#include "gbc_cpu.h"
#include "gbc_gpu.h"
#include "gbc_mmu.h"
#include "gbc_tim.h"
#include "gbc_apu.h"
#include "cmod.h"
#include "sdc.h"

GBC_LoadState_t GBC_LoadState = GBC_LOAD_STATE_NONE; // Global GBC load state

GBC_LoadResult_t GBC_LoadFromCartridge(void)
{
    GBC_Unload();

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
    GBC_TIM_Initialize();
    GBC_GPU_Initialize();
    GBC_APU_Initialize();

    GBC_LoadState = GBC_LOAD_STATE_CARTRIDGE;

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
    GBC_TIM_Initialize();
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

void GBC_SDC_SaveERAM(void)
{
    FIL      file;
    char     path[OS_MAX_PATH_LENGTH];
    uint32_t bytesWritten;
    
    OS_GetSaveGamePath(&OS_CurrentGame, path, OS_MAX_PATH_LENGTH);
    
    f_open(&file, path, FA_OPEN_ALWAYS | FA_WRITE);
    f_write(&file, GBC_MMU_Memory.ERAMBank0, 4 * 8192, &bytesWritten);
    f_close(&file);
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
    // Update until VBlank occurs
    do
    {
        GBC_CPU_Step();
        GBC_TIM_Step();
        GBC_APU_Step();
    }
    while (!GBC_GPU_Step());
    
    // Save RAM after every frame.
    //GBC_SDC_SaveERAM();
}
