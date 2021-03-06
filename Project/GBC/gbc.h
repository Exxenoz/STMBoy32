#ifndef GBC_H
#define GBC_H

#include "common.h"

typedef enum GBC_LoadResult_e
{
    GBC_LOAD_RESULT_OK,
    GBC_LOAD_RESULT_NO_CARTRIDGE,
    GBC_LOAD_RESULT_NO_VALID_CARTRIDGE,
    GBC_LOAD_RESULT_NO_SDC,
    GBC_LOAD_RESULT_NO_ROM_FILE,
    GBC_LOAD_RESULT_NO_VALID_SAV_FILE,
}
GBC_LoadResult_t;

typedef enum GBC_LoadState_e
{
    GBC_LOAD_STATE_NONE,
    GBC_LOAD_STATE_CARTRIDGE,
    GBC_LOAD_STATE_SDC,
}
GBC_LoadState_t;

extern GBC_LoadState_t GBC_LoadState; // External declaration for gbc_mmu.c file

GBC_LoadResult_t GBC_LoadFromCartridge(void);
GBC_LoadResult_t GBC_LoadFromSDC(char* romFile);
GBC_LoadResult_t GBC_SDC_LoadERAM(char *path);
void GBC_SDC_SaveCurrentERAM(void);
void GBC_Unload(void);

bool GBC_IsLoaded(void);
bool GBC_IsLoadedFromCartridge(void);
bool GBC_IsLoadedFromSDC(void);

void GBC_Update(void);

#endif
