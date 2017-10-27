#include "gbc.h"
#include "gbc_cpu.h"
#include "gbc_gpu.h"
#include "gbc_mmu.h"
#include "cmod.h"
#include "sdc.h"

GBC_LoadState_t GBC_LoadState = GBC_LOAD_STATE_NONE; // Global GBC load state

GBC_LoadResult_t GBC_LoadFromCartridge(void)
{
    GBC_Unload();

    CMOD_ResetCartridge();

    if (!CMOD_Detect())
    {
        return GBC_LOAD_RESULT_NO_CARTRIDGE;
    }

    if (!GBC_MMU_LoadFromCartridge())
    {
        return GBC_LOAD_RESULT_NO_VALID_CARTRIDGE;
    }

    GBC_CPU_Initialize();

    GBC_LoadState = GBC_LOAD_STATE_CARTRIDGE;

    return GBC_LOAD_RESULT_OK;
}

GBC_LoadResult_t GBC_LoadFromSDC(char* fileName)
{
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

    GBC_LoadState = GBC_LOAD_STATE_SDC;

    return GBC_LOAD_RESULT_OK;
}

void GBC_Unload(void)
{
    GBC_MMU_Unload();

    GBC_LoadState = GBC_LOAD_STATE_NONE;
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
    // 70224 ticks correspond to exactly one frame
    while (GBC_CPU_Ticks < 70224)
    {
        GBC_CPU_Step();
        GBC_GPU_Step();
    }

    if (GBC_CPU_Ticks >= 70224)
    {
        GBC_CPU_Ticks -= 70224;
    }
}
