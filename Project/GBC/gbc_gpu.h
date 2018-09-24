#ifndef GBC_GPU_H
#define GBC_GPU_H

#include "common.h"

typedef enum GBC_GPU_Mode_e
{
    GBC_GPU_MODE_0_DURING_HBLANK        = 0, // During HBlank period and CPU can access VRAM and OAM
    GBC_GPU_MODE_1_DURING_VBLANK        = 1, // During VBlank period and CPU can access VRAM and OAM
    GBC_GPU_MODE_2_DURING_OAM_READING   = 2, // During reading from OAM, CPU can NOT access OAM
    GBC_GPU_MODE_3_DURING_DATA_TRANSFER = 3, // During reading from OAM and VRAM, CPU can NOT access OAM and VRAM        - GBC mode: Can not accessed Palette Data (FF69, FF6B) too
}
GBC_GPU_Mode_t;

typedef union GBC_GPU_StatusInterruptRequestState_u
{
    uint8_t RequestFlags;

    struct
    {
        unsigned int HBlankInterruptRequest      : 1;
        unsigned int VBlankInterruptRequest      : 1;
        unsigned int OAMInterruptRequest         : 1;
        unsigned int CoincidenceInterruptRequest : 1;
        unsigned int                             : 4;
    };
}
GBC_GPU_StatusInterruptRequestState_t;

#pragma pack(1)
typedef union GBC_GPU_Color_u
{
    uint16_t Color;

    #pragma pack(1)
    struct
    {
        unsigned int Blue  : 5; // LSB
        unsigned int Green : 6;
        unsigned int Red   : 5; // MSB
    };
}
GBC_GPU_Color_t;

#pragma pack(1)
typedef struct GBC_GPU_PriorityPixel_s
{
    #pragma pack(1)
    union
    {
        uint8_t PriorityFlags;

        #pragma pack(1)
        struct
        {
            unsigned int BGPriority        : 1; // BG tile will have priority above all OBJs (regardless of the priority bits in OAM memory)
            unsigned int BGPixel           : 2; // Background pixel color value for sprite priority detection
            unsigned int SpritePositionSet : 1; // True, when a valid sprite position for priority ordering is set
            unsigned int                   : 4;
        };
    };

    uint8_t SpritePositionX;
}
GBC_GPU_PriorityPixel_t;

#define GBC_GPU_FRAME_SIZE_X 160
#define GBC_GPU_FRAME_SIZE_Y 144
#define GBC_GPU_FRAME_SIZE 23040 // GBC_GPU_FRAME_SIZE_X * GBC_GPU_FRAME_SIZE_Y

extern GBC_GPU_Color_t GBC_GPU_FrameBuffer[GBC_GPU_FRAME_SIZE]; // External declaration for LCD access

void GBC_GPU_Initialize(void);
void GBC_GPU_EnableDisplay(void);
void GBC_GPU_DisableDisplay(void);
uint8_t GBC_GPU_FetchBackgroundPaletteColor(uint8_t hl, uint8_t paletteIndex, uint8_t colorIndex);
void GBC_GPU_SetBackgroundPaletteColor(uint8_t hl, uint8_t paletteIndex, uint8_t colorIndex, uint8_t value);
uint8_t GBC_GPU_FetchSpritePaletteColor(uint8_t hl, uint8_t paletteIndex, uint8_t colorIndex);
void GBC_GPU_SetSpritePaletteColor(uint8_t hl, uint8_t paletteIndex, uint8_t colorIndex, uint8_t value);
bool GBC_GPU_Step(void);

#endif
