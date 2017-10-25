#include "gbc.h"
#include "gbc_mmu.h"
#include "sdc.h"

// Global GBC load state
GBC_LoadState_t GBC_LoadState = GBC_LOAD_STATE_NONE;

GBC_LoadResult_t GBC_LoadFromCartridge(void)
{
    // ToDo
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

    GBC_LoadState = GBC_LOAD_STATE_SDC;

    return GBC_LOAD_RESULT_OK;
}

void GBC_Unload(void)
{
    GBC_MMU_Unload();
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
