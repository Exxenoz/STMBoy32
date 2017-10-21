#ifndef GBC_GPU_H
#define GBC_GPU_H

#include "common.h"

enum GBC_GPU_ControlFlags_e
{
    GBC_CPU_CONTROL_FLAGS_NONE         =   0,
    GBC_GPU_CONTROL_FLAGS_BG           =   1, // Off or on
    GBC_GPU_CONTROL_FLAGS_SPRITES      =   2, // Off or on
    GBC_GPU_CONTROL_FLAGS_SPRITES_SIZE =   4, // 8x8 or 8x16
    GBC_GPU_CONTROL_FLAGS_BG_TILEMAP   =   8, // #0 or #1
    GBC_GPU_CONTROL_FLAGS_BG_TILESET   =  16, // #0 or #1
    GBC_GPU_CONTROL_FLAGS_WINDOW       =  32, // Off or on
    GBC_GPU_CONTROL_FLAGS_WINDOW_TM    =  64, // Window Tilemap: Off or on
    GBC_GPU_CONTROL_FLAGS_DISPLAY      = 128, // Off or on
};

#endif
