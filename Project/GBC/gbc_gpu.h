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

typedef union GBC_GPU_Color_e
{
    uint16_t Color;

    struct
    {
        uint8_t Blue  : 5; // LSB
        uint8_t Green : 6;
        uint8_t Red   : 5; // MSB
    };
}
GBC_GPU_Color_t;

extern GBC_GPU_Color_t GBC_GPU_FrameBuffer[160 * 144]; // External declaration for LCD access

void GBC_GPU_Initialize(void);
void GBC_GPU_Step(void);

#endif
