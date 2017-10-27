#ifndef GBC_GPU_H
#define GBC_GPU_H

#include "common.h"

enum GBC_GPU_ControlFlags_e
{
    GBC_CPU_CONTROL_FLAGS_NONE                   =   0,
    GBC_GPU_CONTROL_FLAGS_BG                     =   1, // Off or on                  - GB mode: When cleared, the background becomes blank (white)
                                                        //                            - GBC mode: When cleared, the sprites will be always displayed on top of background and window
    GBC_GPU_CONTROL_FLAGS_SPRITES                =   2, // Off or on
    GBC_GPU_CONTROL_FLAGS_SPRITES_SIZE           =   4, // 8x8 or 8x16
    GBC_GPU_CONTROL_FLAGS_BG_TILEMAP             =   8, // #0 or #1
    GBC_GPU_CONTROL_FLAGS_BG_TILESET             =  16, // #0 or #1
    GBC_GPU_CONTROL_FLAGS_WINDOW                 =  32, // Off or on
    GBC_GPU_CONTROL_FLAGS_WINDOW_TM              =  64, // Window Tilemap: Off or on
    GBC_GPU_CONTROL_FLAGS_DISPLAY                = 128, // Off or on
};

enum GBC_GPU_StatusFlags_e
{
    GBC_GPU_STATUS_MODE_0_DURING_HBLANK          =   0, // During HBlank period and CPU can access VRAM and OAM
    GBC_GPU_STATUS_MODE_1_DURING_VBLANK          =   1, // During VBlank period and CPU can access VRAM and OAM
    GBC_GPU_STATUS_MODE_2_DURING_OAM_READING     =   2, // During reading from OAM, CPU can NOT access OAM
    GBC_GPU_STATUS_MODE_3_DURING_DATA_TRANSFER   =   3, // During reading from OAM and VRAM, CPU can NOT access OAM and VRAM        - GBC mode: Can not accessed Palette Data (FF69, FF6B) too
    GBC_GPU_STATUS_FLAGS_COINCIDENCE             =   4, // 0: Scanline != ScalineCompare, 1: Scanline = ScalineCompare
    GBC_GPU_STATUS_FLAGS_MODE_0_HBLANK_INTERRUPT =   8, // 1: Enable
    GBC_GPU_STATUS_FLAGS_MODE_1_VBLANK_INTERRUPT =  16, // 1: Enable
    GBC_GPU_STATUS_FLAGS_MODE_2_OAM_INTERRUPT    =  32, // 1: Enable
    GBC_GPU_STATUS_FLAGS_COINCIDENCE_INTERRUPT   =  64, // 1: Enable
};

#endif
