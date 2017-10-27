#ifndef GBC_GPU_H
#define GBC_GPU_H

#include "common.h"

enum GBC_GPU_Mode_e
{
    GBC_GPU_MODE_0_DURING_HBLANK        = 0, // During HBlank period and CPU can access VRAM and OAM
    GBC_GPU_MODE_1_DURING_VBLANK        = 1, // During VBlank period and CPU can access VRAM and OAM
    GBC_GPU_MODE_2_DURING_OAM_READING   = 2, // During reading from OAM, CPU can NOT access OAM
    GBC_GPU_MODE_3_DURING_DATA_TRANSFER = 3, // During reading from OAM and VRAM, CPU can NOT access OAM and VRAM        - GBC mode: Can not accessed Palette Data (FF69, FF6B) too
};

void GBC_GPU_Step(void);

#endif
