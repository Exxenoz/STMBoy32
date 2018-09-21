#include "gbc_gpu.h"
#include "gbc_cpu.h"
#include "gbc_mmu.h"
#include "lcd_drawing.h"
#include "string.h"

uint32_t GBC_GPU_ModeTicks = 0;
uint32_t GBC_GPU_Mode = GBC_GPU_MODE_1_DURING_VBLANK; // Needed, because some games overwrite mode
GBC_GPU_StatusInterruptRequestState_t GBC_GPU_StatusInterruptRequestState;
GBC_GPU_Color_t GBC_GPU_FrameBuffer[GBC_GPU_FRAME_SIZE];

#ifdef GBC_GPU_FRAME_RATE_30HZ_MODE
bool GBC_GPU_SkipCurrentFrame = true;
#endif

#pragma pack(1)
typedef struct GBC_GPU_RenderCache_s
{
    uint16_t CurrFrameBufferStartIndex; // Start index of the current scanline
    uint16_t CurrFrameBufferIndex;      // Current render position in the frame buffer
    uint16_t CurrFrameBufferEndIndex;   // End index of the current scanline

    uint16_t CurrScreenOffsetX;         // Which column in the tile map needs to be rendered (ScrollX)
    uint16_t CurrScreenOffsetY;         // Which row in the tile map needs to be rendered (Scanline + ScrollY)

    uint16_t CurrTilePositionX;         // Map column of the current tile
    uint16_t CurrTilePositionY;         // Map row of the current tile

    uint8_t CurrTilePixelPositionX;     // Tile column of the current pixel. Numbered from left to right (0-7)
    uint8_t CurrTilePixelPositionY;     // Tile row of the current pixel.    Numbered from top to bottom (0-7)

    uint16_t CurrTileMapTileOffset;     // Offset to the first tile in the current tile map row (including index offset to TileMap0/1)
    uint16_t CurrTileMapTileIndex;      // Index of the current tile in the tile map data array
    uint16_t CurrTileId;                // Id of the current tile in the tile set data array (of either VRAMBank0 or VRAMBank1)

    union TileMapTileAttributes_u CurrTileMapTileAttributes; // Cache for background and window rendering in CGB mode

    uint16_t CurrTileSetTileIndex;      // Index of the current tile in the tile set data array (of either VRAMBank0 or VRAMBank1)
    uint16_t CurrTilePixelLine;         // Each tile is 8x8 pixel in size and uses 2 bytes per row/line or 2 bits per pixel

    uint16_t CurrPriorityPixelLineIndex;
    GBC_GPU_PriorityPixel_t PriorityPixelLine[GBC_GPU_FRAME_SIZE_X];
    GBC_GPU_PriorityPixel_t CurrPriorityPixel; // Cache for sprite rendering

    uint8_t CurrPixel;

    uint8_t CurrSpriteHeight;
    int32_t CurrSpritePositionX;
    int32_t CurrSpritePositionY;

    struct SpriteAttributes_s CurrSpriteAttributes;
}
GBC_GPU_RenderCache_t;

GBC_GPU_RenderCache_t GBC_GPU_RenderCache;
#define RC GBC_GPU_RenderCache

GBC_GPU_Color_t GBC_GPU_BackgroundPaletteClassic[4] =
{
    0xFFFF, // 0 - White
    0x524A, // 1 - 33% on
    0xA294, // 2 - 66% on
    0x0000, // 3 - Black
};

GBC_GPU_Color_t GBC_GPU_ObjectPalette0Classic[4] =
{
    0xFFFF, // 0 - White
    0x524A, // 1 - 33% on
    0xA294, // 2 - 66% on
    0x0000, // 3 - Black
};

GBC_GPU_Color_t GBC_GPU_ObjectPalette1Classic[4] =
{
    0xFFFF, // 0 - White
    0x524A, // 1 - 33% on
    0xA294, // 2 - 66% on
    0x0000, // 3 - Black
};

GBC_GPU_Color_t GBC_GPU_CGB_BackgroundPalette[8][4];
GBC_GPU_Color_t GBC_GPU_CGB_SpritePalette[8][4];

void GBC_GPU_Initialize(void)
{
    GBC_GPU_ModeTicks = 0;

    // Start in VBlank mode
    GBC_GPU_Mode = GBC_GPU_MODE_1_DURING_VBLANK;
    GBC_MMU_Memory.IO.GPUMode = GBC_GPU_MODE_1_DURING_VBLANK;
    GBC_MMU_Memory.IO.Scanline = 144;

    memset(&GBC_GPU_StatusInterruptRequestState, 0, sizeof(GBC_GPU_StatusInterruptRequestState_t));

    // Frame buffer must not be initialized

#ifdef GBC_GPU_FRAME_RATE_30HZ_MODE
    GBC_GPU_SkipCurrentFrame = true;
#endif

    // Initialize render cache
    memset(&GBC_GPU_RenderCache, 0, sizeof(GBC_GPU_RenderCache_t));

    RC.CurrFrameBufferStartIndex = 0;
    RC.CurrFrameBufferEndIndex = 160;

    for (uint32_t y = 0; y < 8; y++)
    {
        for (uint32_t x = 0; x < 4; x++)
        {
            // Initially all background colors are initialized as white
            GBC_GPU_CGB_BackgroundPalette[y][x].Color = 0xFFFF;
            // Initially all sprite colors are uninitialized
            //GBC_GPU_CGB_SpritePalette -- Do nothing
            // Since the variable is global it is initialized with zeros at the first startup.
        }
    }
}

uint8_t GBC_GPU_FetchBackgroundPaletteColor(uint8_t hl, uint8_t paletteIndex, uint8_t colorIndex)
{
    GBC_GPU_Color_t color = GBC_GPU_CGB_BackgroundPalette[paletteIndex][colorIndex];

    if (hl)
    {
        return (color.Blue << 2) /* Blue */ || ((color.Green >> 3) & 0x3) /* Half Green High */;
    }
    else
    {
        return ((color.Green & 0x7) << 5) /* Half Green Low */ | color.Red /* Red */;
    }
}

void GBC_GPU_SetBackgroundPaletteColor(uint8_t hl, uint8_t paletteIndex, uint8_t colorIndex, uint8_t value)
{
    if (hl)
    {
        GBC_GPU_CGB_BackgroundPalette[paletteIndex][colorIndex].Blue = (value >> 2) & 0x1F /* Blue */;
        GBC_GPU_CGB_BackgroundPalette[paletteIndex][colorIndex].Green = ((value & 0x3) << 3) /* Half Green High */
            | (GBC_GPU_CGB_BackgroundPalette[paletteIndex][colorIndex].Green & 0x7) /* Half Green Low */;
    }
    else
    {
        GBC_GPU_CGB_BackgroundPalette[paletteIndex][colorIndex].Red = value & 0x1F /* Red */;
        GBC_GPU_CGB_BackgroundPalette[paletteIndex][colorIndex].Green = (GBC_GPU_CGB_BackgroundPalette[paletteIndex][colorIndex].Green & 0x18) /* Half Green High */
            | ((value >> 5) & 0x7) /* Half Green Low */;
    }
}

uint8_t GBC_GPU_FetchSpritePaletteColor(uint8_t hl, uint8_t paletteIndex, uint8_t colorIndex)
{
    GBC_GPU_Color_t color = GBC_GPU_CGB_SpritePalette[paletteIndex][colorIndex];

    if (hl)
    {
        return (color.Blue << 2) /* Blue */ || ((color.Green >> 3) & 0x3) /* Half Green High */;
    }
    else
    {
        return ((color.Green & 0x7) << 5) /* Half Green Low */ | color.Red /* Red */;
    }
}

void GBC_GPU_SetSpritePaletteColor(uint8_t hl, uint8_t paletteIndex, uint8_t colorIndex, uint8_t value)
{
    if (hl)
    {
        GBC_GPU_CGB_SpritePalette[paletteIndex][colorIndex].Blue = (value >> 2) & 0x1F /* Blue */;
        GBC_GPU_CGB_SpritePalette[paletteIndex][colorIndex].Green = ((value & 0x3) << 3) /* Half Green High */
            | (GBC_GPU_CGB_SpritePalette[paletteIndex][colorIndex].Green & 0x7) /* Half Green Low */;
    }
    else
    {
        GBC_GPU_CGB_SpritePalette[paletteIndex][colorIndex].Red = value & 0x1F /* Red */;
        GBC_GPU_CGB_SpritePalette[paletteIndex][colorIndex].Green = (GBC_GPU_CGB_SpritePalette[paletteIndex][colorIndex].Green & 0x18) /* Half Green High */
            | ((value >> 5) & 0x7) /* Half Green Low */;
    }
}

static inline void GBC_GPU_DMG_RenderBackgroundScanline(void)
{
    RC.CurrFrameBufferIndex = RC.CurrFrameBufferStartIndex;
    RC.CurrScreenOffsetX = GBC_MMU_Memory.IO.ScrollX;
    RC.CurrScreenOffsetY = GBC_MMU_Memory.IO.Scanline + GBC_MMU_Memory.IO.ScrollY;
    RC.CurrTilePositionX = RC.CurrScreenOffsetX >> 3;
    RC.CurrTilePositionY = (RC.CurrScreenOffsetY & 0xFF) >> 3;
    RC.CurrTilePixelPositionX = RC.CurrScreenOffsetX & 7;
    RC.CurrTilePixelPositionY = RC.CurrScreenOffsetY & 7;
    RC.CurrTileMapTileOffset = (GBC_MMU_Memory.IO.BGTileMapDisplaySelect ? 1024 : 0) + (RC.CurrTilePositionY << 5);
    RC.CurrTileMapTileIndex = RC.CurrTileMapTileOffset + RC.CurrTilePositionX;

    for (RC.CurrPriorityPixelLineIndex = 0; RC.CurrFrameBufferIndex < RC.CurrFrameBufferEndIndex;)
    {
        RC.CurrTileId = GBC_MMU_Memory.VRAMBank0.TileMapData[RC.CurrTileMapTileIndex];

        // Calculate the real tile id if current tile id is signed due to tile set #0 selection
        if (!GBC_MMU_Memory.IO.BGAndWindowTileSetDisplaySelect && RC.CurrTileId < 128)
        {
            RC.CurrTileId += 256;
        }

        RC.CurrTileSetTileIndex = (RC.CurrTileId << 3) + RC.CurrTilePixelPositionY;
        RC.CurrTilePixelLine = GBC_MMU_Memory.VRAMBank0.TileSetData[RC.CurrTileSetTileIndex];

        // Move first pixel bits to bit 15 (low) and bit 7 (high)
        RC.CurrTilePixelLine <<= RC.CurrTilePixelPositionX;

        for (; RC.CurrFrameBufferIndex < RC.CurrFrameBufferEndIndex && RC.CurrTilePixelPositionX < 8;
            RC.CurrTilePixelPositionX++, RC.CurrPriorityPixelLineIndex++)
        {
            RC.CurrPixel = ((RC.CurrTilePixelLine & 0x80) >> 6) | ((RC.CurrTilePixelLine & 0x8000) >> 15);

            // Save background pixel value for sprite priority system
            RC.PriorityPixelLine[RC.CurrPriorityPixelLineIndex].BGPixel = RC.CurrPixel;

            switch (RC.CurrPixel)
            {
                case 0:
                    GBC_GPU_FrameBuffer[RC.CurrFrameBufferIndex++] = GBC_GPU_BackgroundPaletteClassic[GBC_MMU_Memory.IO.BackgroundPaletteColor0];
                    break;
                case 1:
                    GBC_GPU_FrameBuffer[RC.CurrFrameBufferIndex++] = GBC_GPU_BackgroundPaletteClassic[GBC_MMU_Memory.IO.BackgroundPaletteColor1];
                    break;
                case 2:
                    GBC_GPU_FrameBuffer[RC.CurrFrameBufferIndex++] = GBC_GPU_BackgroundPaletteClassic[GBC_MMU_Memory.IO.BackgroundPaletteColor2];
                    break;
                case 3:
                    GBC_GPU_FrameBuffer[RC.CurrFrameBufferIndex++] = GBC_GPU_BackgroundPaletteClassic[GBC_MMU_Memory.IO.BackgroundPaletteColor3];
                    break;
            }

            RC.CurrTilePixelLine <<= 1;
        }

        RC.CurrTilePixelPositionX = 0;

        RC.CurrTilePositionX++;
        RC.CurrTilePositionX &= 0x1F;

        RC.CurrTileMapTileIndex = RC.CurrTileMapTileOffset + RC.CurrTilePositionX;
    }
}

static inline void GBC_GPU_CGB_RenderBackgroundScanline(void)
{
    RC.CurrFrameBufferIndex = RC.CurrFrameBufferStartIndex;
    RC.CurrScreenOffsetX = GBC_MMU_Memory.IO.ScrollX;
    RC.CurrScreenOffsetY = GBC_MMU_Memory.IO.Scanline + GBC_MMU_Memory.IO.ScrollY;
    RC.CurrTilePositionX = RC.CurrScreenOffsetX >> 3;
    RC.CurrTilePositionY = (RC.CurrScreenOffsetY & 0xFF) >> 3;
    RC.CurrTilePixelPositionX = RC.CurrScreenOffsetX & 7;
    RC.CurrTilePixelPositionY = RC.CurrScreenOffsetY & 7;
    RC.CurrTileMapTileOffset = (GBC_MMU_Memory.IO.BGTileMapDisplaySelect ? 1024 : 0) + (RC.CurrTilePositionY << 5);
    RC.CurrTileMapTileIndex = RC.CurrTileMapTileOffset + RC.CurrTilePositionX;

    for (RC.CurrPriorityPixelLineIndex = 0; RC.CurrFrameBufferIndex < RC.CurrFrameBufferEndIndex;)
    {
        RC.CurrTileId = GBC_MMU_Memory.VRAMBank0.TileMapData[RC.CurrTileMapTileIndex];

        // Calculate the real tile id if current tile id is signed due to tile set #0 selection
        if (!GBC_MMU_Memory.IO.BGAndWindowTileSetDisplaySelect && RC.CurrTileId < 128)
        {
            RC.CurrTileId += 256;
        }

        RC.CurrTileMapTileAttributes = GBC_MMU_Memory.VRAMBank1.TileMapTileAttributes[RC.CurrTileMapTileIndex];

        if (RC.CurrTileMapTileAttributes.VerticalFlip)
        {
            RC.CurrTilePixelPositionY = 7 - (RC.CurrScreenOffsetY & 7);
        }
        else
        {
            RC.CurrTilePixelPositionY = RC.CurrScreenOffsetY & 7;
        }

        RC.CurrTileSetTileIndex = (RC.CurrTileId << 3) + RC.CurrTilePixelPositionY;

        if (RC.CurrTileMapTileAttributes.TileVRAMBankNumber)
        {
            RC.CurrTilePixelLine = GBC_MMU_Memory.VRAMBank1.TileSetData[RC.CurrTileSetTileIndex];
        }
        else
        {
            RC.CurrTilePixelLine = GBC_MMU_Memory.VRAMBank0.TileSetData[RC.CurrTileSetTileIndex];
        }

        if (RC.CurrTileMapTileAttributes.HorizontalFlip)
        {
            // Move first pixel bits to bit 8 (low) and bit 0 (high)
            RC.CurrTilePixelLine >>= RC.CurrTilePixelPositionX;
        }
        else
        {
            // Move first pixel bits to bit 15 (low) and bit 7 (high)
            RC.CurrTilePixelLine <<= RC.CurrTilePixelPositionX;
        }

        for (; RC.CurrFrameBufferIndex < RC.CurrFrameBufferEndIndex && RC.CurrTilePixelPositionX < 8;
            RC.CurrTilePixelPositionX++, RC.CurrPriorityPixelLineIndex++, RC.CurrFrameBufferIndex++)
        {
            if (RC.CurrTileMapTileAttributes.HorizontalFlip)
            {
                RC.CurrPixel = ((RC.CurrTilePixelLine & 0x1) << 1) | ((RC.CurrTilePixelLine & 0x100) >> 8);

                // Move next two bits to bit 8 (low) and bit 0 (high)
                RC.CurrTilePixelLine >>= 1;
            }
            else
            {
                RC.CurrPixel = ((RC.CurrTilePixelLine & 0x80) >> 6) | ((RC.CurrTilePixelLine & 0x8000) >> 15);

                // Move next two bits to bit 15 (low) and bit 7 (high)
                RC.CurrTilePixelLine <<= 1;
            }

            // Save background pixel value for sprite priority system
            RC.PriorityPixelLine[RC.CurrPriorityPixelLineIndex].BGPixel = RC.CurrPixel;
            RC.PriorityPixelLine[RC.CurrPriorityPixelLineIndex].BGPriority = RC.CurrTileMapTileAttributes.BGOAMPriority;

            GBC_GPU_FrameBuffer[RC.CurrFrameBufferIndex] = GBC_GPU_CGB_BackgroundPalette[RC.CurrTileMapTileAttributes.BackgroundPaletteIndex][RC.CurrPixel];
            GBC_GPU_FrameBuffer[RC.CurrFrameBufferIndex].Green <<= 1;
        }

        RC.CurrTilePixelPositionX = 0;

        RC.CurrTilePositionX++;
        RC.CurrTilePositionX &= 0x1F;

        RC.CurrTileMapTileIndex = RC.CurrTileMapTileOffset + RC.CurrTilePositionX;
    }
}

static inline void GBC_GPU_DMG_RenderWindowScanline(void)
{
    if (GBC_MMU_Memory.IO.WindowPositionXMinus7 > 7)
    {
        RC.CurrFrameBufferIndex = RC.CurrFrameBufferStartIndex + GBC_MMU_Memory.IO.WindowPositionXMinus7 - 7;
    }
    else
    {
        RC.CurrFrameBufferIndex = RC.CurrFrameBufferStartIndex;
    }

    RC.CurrScreenOffsetX = 0;
    RC.CurrScreenOffsetY = GBC_MMU_Memory.IO.Scanline - GBC_MMU_Memory.IO.WindowPositionY;
    RC.CurrTilePositionX = 0;
    RC.CurrTilePositionY = (RC.CurrScreenOffsetY & 0xFF) >> 3;
    RC.CurrTilePixelPositionX = (GBC_MMU_Memory.IO.WindowPositionXMinus7 < 7) ? 7 - GBC_MMU_Memory.IO.WindowPositionXMinus7 : 0;
    RC.CurrTilePixelPositionY = RC.CurrScreenOffsetY & 7;
    RC.CurrTileMapTileIndex = (RC.CurrTilePositionY << 5) + RC.CurrTilePositionX;

    // Check if tile map #2 is selected
    if (GBC_MMU_Memory.IO.WindowTileMapDisplaySelect)
    {
        RC.CurrTileMapTileIndex += 1024; // Use tile map #2
    }

    for (RC.CurrPriorityPixelLineIndex = 0; RC.CurrFrameBufferIndex < RC.CurrFrameBufferEndIndex; RC.CurrTileMapTileIndex++)
    {
        RC.CurrTileId = GBC_MMU_Memory.VRAMBank0.TileMapData[RC.CurrTileMapTileIndex];

        // Calculate the real tile id if current tile id is signed due to tile set #0 selection
        if (!GBC_MMU_Memory.IO.BGAndWindowTileSetDisplaySelect && RC.CurrTileId < 128)
        {
            RC.CurrTileId += 256;
        }

        RC.CurrTileSetTileIndex = (RC.CurrTileId << 3) + RC.CurrTilePixelPositionY;
        RC.CurrTilePixelLine = GBC_MMU_Memory.VRAMBank0.TileSetData[RC.CurrTileSetTileIndex];

        // Move first pixel bits to bit 15 (low) and bit 7 (high)
        RC.CurrTilePixelLine <<= RC.CurrTilePixelPositionX;

        for (; RC.CurrFrameBufferIndex < RC.CurrFrameBufferEndIndex && RC.CurrTilePixelPositionX < 8;
            RC.CurrTilePixelPositionX++, RC.CurrPriorityPixelLineIndex++)
        {
            RC.CurrPixel = ((RC.CurrTilePixelLine & 0x80) >> 6) | ((RC.CurrTilePixelLine & 0x8000) >> 15);

            // Save window pixel value for sprite priority system
            RC.PriorityPixelLine[RC.CurrPriorityPixelLineIndex].BGPixel = RC.CurrPixel;

            switch (RC.CurrPixel)
            {
                case 0:
                    GBC_GPU_FrameBuffer[RC.CurrFrameBufferIndex++] = GBC_GPU_BackgroundPaletteClassic[GBC_MMU_Memory.IO.BackgroundPaletteColor0];
                    break;
                case 1:
                    GBC_GPU_FrameBuffer[RC.CurrFrameBufferIndex++] = GBC_GPU_BackgroundPaletteClassic[GBC_MMU_Memory.IO.BackgroundPaletteColor1];
                    break;
                case 2:
                    GBC_GPU_FrameBuffer[RC.CurrFrameBufferIndex++] = GBC_GPU_BackgroundPaletteClassic[GBC_MMU_Memory.IO.BackgroundPaletteColor2];
                    break;
                case 3:
                    GBC_GPU_FrameBuffer[RC.CurrFrameBufferIndex++] = GBC_GPU_BackgroundPaletteClassic[GBC_MMU_Memory.IO.BackgroundPaletteColor3];
                    break;
            }

            RC.CurrTilePixelLine <<= 1;
        }

        RC.CurrTilePixelPositionX = 0;
    }
}

static inline void GBC_GPU_CGB_RenderWindowScanline(void)
{
    if (GBC_MMU_Memory.IO.WindowPositionXMinus7 > 7)
    {
        RC.CurrFrameBufferIndex = RC.CurrFrameBufferStartIndex + GBC_MMU_Memory.IO.WindowPositionXMinus7 - 7;
    }
    else
    {
        RC.CurrFrameBufferIndex = RC.CurrFrameBufferStartIndex;
    }

    RC.CurrScreenOffsetX = 0;
    RC.CurrScreenOffsetY = GBC_MMU_Memory.IO.Scanline - GBC_MMU_Memory.IO.WindowPositionY;
    RC.CurrTilePositionX = 0;
    RC.CurrTilePositionY = (RC.CurrScreenOffsetY & 0xFF) >> 3;
    RC.CurrTilePixelPositionX = (GBC_MMU_Memory.IO.WindowPositionXMinus7 < 7) ? 7 - GBC_MMU_Memory.IO.WindowPositionXMinus7 : 0;
    RC.CurrTilePixelPositionY = RC.CurrScreenOffsetY & 7;
    RC.CurrTileMapTileIndex = (RC.CurrTilePositionY << 5) + RC.CurrTilePositionX;

    // Check if tile map #2 is selected
    if (GBC_MMU_Memory.IO.WindowTileMapDisplaySelect)
    {
        RC.CurrTileMapTileIndex += 1024; // Use tile map #2
    }

    for (RC.CurrPriorityPixelLineIndex = 0; RC.CurrFrameBufferIndex < RC.CurrFrameBufferEndIndex; RC.CurrTileMapTileIndex++)
    {
        RC.CurrTileId = GBC_MMU_Memory.VRAMBank0.TileMapData[RC.CurrTileMapTileIndex];

        // Calculate the real tile id if current tile id is signed due to tile set #0 selection
        if (!GBC_MMU_Memory.IO.BGAndWindowTileSetDisplaySelect && RC.CurrTileId < 128)
        {
            RC.CurrTileId += 256;
        }

        RC.CurrTileMapTileAttributes = GBC_MMU_Memory.VRAMBank1.TileMapTileAttributes[RC.CurrTileMapTileIndex];

        if (RC.CurrTileMapTileAttributes.VerticalFlip)
        {
            RC.CurrTilePixelPositionY = 7 - (RC.CurrScreenOffsetY & 7);
        }
        else
        {
            RC.CurrTilePixelPositionY = RC.CurrScreenOffsetY & 7;
        }

        RC.CurrTileSetTileIndex = (RC.CurrTileId << 3) + RC.CurrTilePixelPositionY;

        if (RC.CurrTileMapTileAttributes.TileVRAMBankNumber)
        {
            RC.CurrTilePixelLine = GBC_MMU_Memory.VRAMBank1.TileSetData[RC.CurrTileSetTileIndex];
        }
        else
        {
            RC.CurrTilePixelLine = GBC_MMU_Memory.VRAMBank0.TileSetData[RC.CurrTileSetTileIndex];
        }

        if (RC.CurrTileMapTileAttributes.HorizontalFlip)
        {
            // Move first pixel bits to bit 8 (low) and bit 0 (high)
            RC.CurrTilePixelLine >>= RC.CurrTilePixelPositionX;
        }
        else
        {
            // Move first pixel bits to bit 15 (low) and bit 7 (high)
            RC.CurrTilePixelLine <<= RC.CurrTilePixelPositionX;
        }

        for (; RC.CurrFrameBufferIndex < RC.CurrFrameBufferEndIndex && RC.CurrTilePixelPositionX < 8;
            RC.CurrTilePixelPositionX++, RC.CurrPriorityPixelLineIndex++, RC.CurrFrameBufferIndex++)
        {
            if (RC.CurrTileMapTileAttributes.HorizontalFlip)
            {
                RC.CurrPixel = ((RC.CurrTilePixelLine & 0x1) << 1) | ((RC.CurrTilePixelLine & 0x100) >> 8);

                // Move next two bits to bit 8 (low) and bit 0 (high)
                RC.CurrTilePixelLine >>= 1;
            }
            else
            {
                RC.CurrPixel = ((RC.CurrTilePixelLine & 0x80) >> 6) | ((RC.CurrTilePixelLine & 0x8000) >> 15);

                // Move next two bits to bit 15 (low) and bit 7 (high)
                RC.CurrTilePixelLine <<= 1;
            }

            // Save background pixel value for sprite priority system
            RC.PriorityPixelLine[RC.CurrPriorityPixelLineIndex].BGPixel = RC.CurrPixel;
            RC.PriorityPixelLine[RC.CurrPriorityPixelLineIndex].BGPriority = RC.CurrTileMapTileAttributes.BGOAMPriority;

            GBC_GPU_FrameBuffer[RC.CurrFrameBufferIndex] = GBC_GPU_CGB_BackgroundPalette[RC.CurrTileMapTileAttributes.BackgroundPaletteIndex][RC.CurrPixel];
            GBC_GPU_FrameBuffer[RC.CurrFrameBufferIndex].Green <<= 1;
        }

        RC.CurrTilePixelPositionX = 0;
    }
}

static inline void GBC_GPU_DMG_RenderSpriteScanline(void)
{
    RC.CurrSpriteHeight = GBC_MMU_Memory.IO.SpriteSize ? 16 : 8;

    for (int32_t i = 39; i >= 0; i--)
    {
        RC.CurrSpriteAttributes = GBC_MMU_Memory.OAM.SpriteAttributes[i];

        /* Sprite positioning:
         *
         *  <--  8px  -->
         *  1 * * * * * * ^
         *  * ~~~~~~~~~~* |
         *  *  +++  +++ * | 8px
         *  *  ===  === * |    /
         *  *     ==    * |     16px
         *  *  =      = * |
         *  *   ======  * |
         *  * * * * * * * v
         *                0 ... Original  Pivot
         *                1 ... Corrected Pivot
         */

        RC.CurrSpritePositionX = RC.CurrSpriteAttributes.PositionX -  8;
        RC.CurrSpritePositionY = RC.CurrSpriteAttributes.PositionY - 16;

        // Check if sprite falls on this scanline
        if (RC.CurrSpritePositionY > GBC_MMU_Memory.IO.Scanline ||
           (RC.CurrSpritePositionY + RC.CurrSpriteHeight) <= GBC_MMU_Memory.IO.Scanline ||
            RC.CurrSpritePositionX <=  -8 ||
            RC.CurrSpritePositionX >= 160)
        {
            continue;
        }

        if (RC.CurrSpriteHeight == 16)
        {
            // In 8x16 mode, the lower bit of the tile number is ignored.
            RC.CurrTileId = RC.CurrSpriteAttributes.TileID & 0xFE;
        }
        else
        {
            RC.CurrTileId = RC.CurrSpriteAttributes.TileID;
        }

        if (RC.CurrSpriteAttributes.FlipY)
        {
            if (RC.CurrSpriteHeight == 16)
            {
                RC.CurrTilePixelPositionY = 15 - (GBC_MMU_Memory.IO.Scanline - RC.CurrSpritePositionY);

                // Check if pixel line is in the second tile
                if (RC.CurrTilePixelPositionY >= 8)
                {
                    RC.CurrTilePixelPositionY -= 8;

                    // Use second tile
                    RC.CurrTileId++;
                }
            }
            else
            {
                RC.CurrTilePixelPositionY = 7 - (GBC_MMU_Memory.IO.Scanline - RC.CurrSpritePositionY);
            }
        }
        else
        {
            RC.CurrTilePixelPositionY = GBC_MMU_Memory.IO.Scanline - RC.CurrSpritePositionY;
        }

        if (RC.CurrSpritePositionX < 0)
        {
            // Start with the first visible pixel
            RC.CurrTilePixelPositionX = 0 - RC.CurrSpritePositionX;
            // Start with the first pixel in the line
            RC.CurrFrameBufferIndex = RC.CurrFrameBufferStartIndex;
        }
        else
        {
            // Start with the first pixel
            RC.CurrTilePixelPositionX = 0;
            // Start line drawing where the first sprite pixel is located
            RC.CurrFrameBufferIndex = RC.CurrFrameBufferStartIndex + RC.CurrSpritePositionX;
        }

        RC.CurrTileSetTileIndex = (RC.CurrTileId << 3) + RC.CurrTilePixelPositionY;
        RC.CurrTilePixelLine = GBC_MMU_Memory.VRAMBank0.TileSetData[RC.CurrTileSetTileIndex];

        if (RC.CurrSpriteAttributes.FlipX)
        {
            // Move first pixel bits to bit 8 (low) and bit 0 (high)
            RC.CurrTilePixelLine >>= RC.CurrTilePixelPositionX;
        }
        else
        {
            // Move first pixel bits to bit 15 (low) and bit 7 (high)
            RC.CurrTilePixelLine <<= RC.CurrTilePixelPositionX;
        }

        for (RC.CurrPriorityPixelLineIndex = RC.CurrFrameBufferIndex - RC.CurrFrameBufferStartIndex;
            RC.CurrTilePixelPositionX < 8 && RC.CurrFrameBufferIndex < RC.CurrFrameBufferEndIndex;
            RC.CurrTilePixelPositionX++, RC.CurrPriorityPixelLineIndex++, RC.CurrFrameBufferIndex++)
        {
            if (RC.CurrSpriteAttributes.FlipX)
            {
                RC.CurrPixel = ((RC.CurrTilePixelLine & 0x1) << 1) | ((RC.CurrTilePixelLine & 0x100) >> 8);

                // Move next two bits to bit 8 (low) and bit 0 (high)
                RC.CurrTilePixelLine >>= 1;
            }
            else
            {
                RC.CurrPixel = ((RC.CurrTilePixelLine & 0x80) >> 6) | ((RC.CurrTilePixelLine & 0x8000) >> 15);

                // Move next two bits to bit 15 (low) and bit 7 (high)
                RC.CurrTilePixelLine <<= 1;
            }

            if (RC.CurrPixel == 0)
            {
                // Skip pixel, because 0 means transparent
                continue;
            }

            RC.CurrPriorityPixel = RC.PriorityPixelLine[RC.CurrPriorityPixelLineIndex];

            if (RC.CurrPriorityPixel.SpritePositionSet)
            {
                // When sprites with different x coordinate values overlap,
                // the one with the smaller x coordinate (closer to the left)
                // will have priority and appear above any others.
                if (RC.CurrSpritePositionX < RC.CurrPriorityPixel.SpritePositionX)
                {
                    RC.CurrPriorityPixel.SpritePositionX = RC.CurrSpritePositionX;
                    RC.PriorityPixelLine[RC.CurrPriorityPixelLineIndex] = RC.CurrPriorityPixel;
                }
                else // Skip this pixel
                {
                    continue;
                }
            }
            else // Set new priority pixel
            {
                RC.CurrPriorityPixel.SpritePositionSet = 1;
                RC.CurrPriorityPixel.SpritePositionX = RC.CurrSpritePositionX;
                RC.PriorityPixelLine[RC.CurrPriorityPixelLineIndex] = RC.CurrPriorityPixel;
            }

            // When background priority is activated, then the sprite is
            // behind BG color 1-3, BG color 0 is always behind object.
            if (RC.CurrSpriteAttributes.BGRenderPriority && RC.CurrPriorityPixel.BGPixel)
            {
                continue;
            }

            // Classic GB mode only
            if (RC.CurrSpriteAttributes.PaletteNumberClassic)
            {
                // Select color from object palette 1
                switch (RC.CurrPixel)
                {
                    case 0:
                        GBC_GPU_FrameBuffer[RC.CurrFrameBufferIndex] = GBC_GPU_ObjectPalette1Classic[GBC_MMU_Memory.IO.ObjectPalette1Color0];
                        break;
                    case 1:
                        GBC_GPU_FrameBuffer[RC.CurrFrameBufferIndex] = GBC_GPU_ObjectPalette1Classic[GBC_MMU_Memory.IO.ObjectPalette1Color1];
                        break;
                    case 2:
                        GBC_GPU_FrameBuffer[RC.CurrFrameBufferIndex] = GBC_GPU_ObjectPalette1Classic[GBC_MMU_Memory.IO.ObjectPalette1Color2];
                        break;
                    case 3:
                        GBC_GPU_FrameBuffer[RC.CurrFrameBufferIndex] = GBC_GPU_ObjectPalette1Classic[GBC_MMU_Memory.IO.ObjectPalette1Color3];
                        break;
                }
            }
            else switch (RC.CurrPixel)
            {
                // Select color from object palette 0
                case 0:
                    GBC_GPU_FrameBuffer[RC.CurrFrameBufferIndex] = GBC_GPU_ObjectPalette0Classic[GBC_MMU_Memory.IO.ObjectPalette0Color0];
                    break;
                case 1:
                    GBC_GPU_FrameBuffer[RC.CurrFrameBufferIndex] = GBC_GPU_ObjectPalette0Classic[GBC_MMU_Memory.IO.ObjectPalette0Color1];
                    break;
                case 2:
                    GBC_GPU_FrameBuffer[RC.CurrFrameBufferIndex] = GBC_GPU_ObjectPalette0Classic[GBC_MMU_Memory.IO.ObjectPalette0Color2];
                    break;
                case 3:
                    GBC_GPU_FrameBuffer[RC.CurrFrameBufferIndex] = GBC_GPU_ObjectPalette0Classic[GBC_MMU_Memory.IO.ObjectPalette0Color3];
                    break;
            }
        }
    }
}

static inline void GBC_GPU_CGB_RenderSpriteScanline(void)
{
    RC.CurrSpriteHeight = GBC_MMU_Memory.IO.SpriteSize ? 16 : 8;

    // When sprites with the same x coordinate values overlap,
    // they have priority according to table ordering.
    // (i.e. $FE00 - highest, $FE04 - next highest, etc.)
    // Start from behind, because the first sprites
    // have the highest render priority in GBC mode
    for (int32_t i = 39; i >= 0; i--)
    {
        RC.CurrSpriteAttributes = GBC_MMU_Memory.OAM.SpriteAttributes[i];

        /* Sprite positioning:
         *
         *  <--  8px  -->
         *  1 * * * * * * ^
         *  * ~~~~~~~~~~* |
         *  *  +++  +++ * | 8px
         *  *  ===  === * |    /
         *  *     ==    * |     16px
         *  *  =      = * |
         *  *   ======  * |
         *  * * * * * * * v
         *                0 ... Original  Pivot
         *                1 ... Corrected Pivot
         */

        RC.CurrSpritePositionX = RC.CurrSpriteAttributes.PositionX -  8;
        RC.CurrSpritePositionY = RC.CurrSpriteAttributes.PositionY - 16;

        // Check if sprite falls on this scanline
        if (RC.CurrSpritePositionY > GBC_MMU_Memory.IO.Scanline ||
           (RC.CurrSpritePositionY + RC.CurrSpriteHeight) <= GBC_MMU_Memory.IO.Scanline ||
            RC.CurrSpritePositionX <=  -8 ||
            RC.CurrSpritePositionX >= 160)
        {
            continue;
        }

        if (RC.CurrSpriteHeight == 16)
        {
            // In 8x16 mode, the lower bit of the tile number is ignored.
            RC.CurrTileId = RC.CurrSpriteAttributes.TileID & 0xFE;
        }
        else
        {
            RC.CurrTileId = RC.CurrSpriteAttributes.TileID;
        }

        if (RC.CurrSpriteAttributes.FlipY)
        {
            if (RC.CurrSpriteHeight == 16)
            {
                RC.CurrTilePixelPositionY = 15 - (GBC_MMU_Memory.IO.Scanline - RC.CurrSpritePositionY);

                // Check if pixel line is in the second tile
                if (RC.CurrTilePixelPositionY >= 8)
                {
                    RC.CurrTilePixelPositionY -= 8;

                    // Use second tile
                    RC.CurrTileId++;
                }
            }
            else
            {
                RC.CurrTilePixelPositionY = 7 - (GBC_MMU_Memory.IO.Scanline - RC.CurrSpritePositionY);
            }
        }
        else
        {
            RC.CurrTilePixelPositionY = GBC_MMU_Memory.IO.Scanline - RC.CurrSpritePositionY;
        }

        if (RC.CurrSpritePositionX < 0)
        {
            // Start with the first visible pixel
            RC.CurrTilePixelPositionX = 0 - RC.CurrSpritePositionX;
            // Start with the first pixel in the line
            RC.CurrFrameBufferIndex = RC.CurrFrameBufferStartIndex;
        }
        else
        {
            // Start with the first pixel
            RC.CurrTilePixelPositionX = 0;
            // Start line drawing where the first sprite pixel is located
            RC.CurrFrameBufferIndex = RC.CurrFrameBufferStartIndex + RC.CurrSpritePositionX;
        }

        RC.CurrTileSetTileIndex = (RC.CurrTileId << 3) + RC.CurrTilePixelPositionY;

        if (RC.CurrSpriteAttributes.TileVRAMBank)
        {
            RC.CurrTilePixelLine = GBC_MMU_Memory.VRAMBank1.TileSetData[RC.CurrTileSetTileIndex];
        }
        else
        {
            RC.CurrTilePixelLine = GBC_MMU_Memory.VRAMBank0.TileSetData[RC.CurrTileSetTileIndex];
        }

        if (RC.CurrSpriteAttributes.FlipX)
        {
            // Move first pixel bits to bit 8 (low) and bit 0 (high)
            RC.CurrTilePixelLine >>= RC.CurrTilePixelPositionX;
        }
        else
        {
            // Move first pixel bits to bit 15 (low) and bit 7 (high)
            RC.CurrTilePixelLine <<= RC.CurrTilePixelPositionX;
        }

        for (RC.CurrPriorityPixelLineIndex = RC.CurrFrameBufferIndex - RC.CurrFrameBufferStartIndex;
            RC.CurrTilePixelPositionX < 8 && RC.CurrFrameBufferIndex < RC.CurrFrameBufferEndIndex;
            RC.CurrTilePixelPositionX++, RC.CurrPriorityPixelLineIndex++, RC.CurrFrameBufferIndex++)
        {
            if (RC.CurrSpriteAttributes.FlipX)
            {
                RC.CurrPixel = ((RC.CurrTilePixelLine & 0x1) << 1) | ((RC.CurrTilePixelLine & 0x100) >> 8);

                // Move next two bits to bit 8 (low) and bit 0 (high)
                RC.CurrTilePixelLine >>= 1;
            }
            else
            {
                RC.CurrPixel = ((RC.CurrTilePixelLine & 0x80) >> 6) | ((RC.CurrTilePixelLine & 0x8000) >> 15);

                // Move next two bits to bit 15 (low) and bit 7 (high)
                RC.CurrTilePixelLine <<= 1;
            }

            if (RC.CurrPixel == 0)
            {
                // Skip pixel, because 0 means transparent
                continue;
            }

            RC.CurrPriorityPixel = RC.PriorityPixelLine[RC.CurrPriorityPixelLineIndex];

            // When BGPriority is set, the corresponding BG tile will have priority
            // above all OBJs (regardless of the priority bits in OAM memory).
            if (RC.CurrPriorityPixel.BGPriority)
            {
                continue;
            }

            // When background priority is activated, then the sprite is
            // behind BG color 1-3, BG color 0 is always behind object.
            if (RC.CurrSpriteAttributes.BGRenderPriority && RC.CurrPriorityPixel.BGPixel)
            {
                continue;
            }

            GBC_GPU_FrameBuffer[RC.CurrFrameBufferIndex] = GBC_GPU_CGB_SpritePalette[RC.CurrSpriteAttributes.PaletteNumber][RC.CurrPixel];
            GBC_GPU_FrameBuffer[RC.CurrFrameBufferIndex].Green <<= 1;
        }
    }
}

void GBC_GPU_RenderScanline(void)
{
    // When the display is disabled the screen is blank (white)
    if (!GBC_MMU_Memory.IO.DisplayEnable)
    {
        memset(&GBC_GPU_FrameBuffer[RC.CurrFrameBufferStartIndex],
            GBC_GPU_BackgroundPaletteClassic[0].Color, sizeof(GBC_GPU_Color_t) * GBC_GPU_FRAME_SIZE_X);
        return;
    }

    // Reset priority pixel line
    memset(&RC.PriorityPixelLine, 0, sizeof(GBC_GPU_PriorityPixel_t) * GBC_GPU_FRAME_SIZE_X);

    // In GBC mode the BGDisplayEnable flag is ignored and sprites are always displayed on top
    if (GBC_MMU_IS_CGB_MODE())
    {
        GBC_GPU_CGB_RenderBackgroundScanline();
    }
    else if (GBC_MMU_Memory.IO.BGDisplayEnable)
    {
        GBC_GPU_DMG_RenderBackgroundScanline();
    }
    // When BGDisplayEnable is false and we are not in GBC mode, the background becomes blank (white).
    else
    {
        memset(&GBC_GPU_FrameBuffer[RC.CurrFrameBufferStartIndex],
            GBC_GPU_BackgroundPaletteClassic[0].Color, sizeof(GBC_GPU_Color_t) * GBC_GPU_FRAME_SIZE_X);
    }

    // The window becomes visible (if enabled) when positions are set in range WX = 0-166, WY = 0-143
    if (GBC_MMU_Memory.IO.WindowDisplayEnable &&
        //GBC_MMU_Memory.WindowPositionY <= 143 && -- Not needed, because Scanline should be in range 0-143 at this position
        GBC_MMU_Memory.IO.WindowPositionY <= GBC_MMU_Memory.IO.Scanline &&
        GBC_MMU_Memory.IO.WindowPositionXMinus7 <= 166) // X = 7 and Y = 0 locates window at upper left
    {
        if (GBC_MMU_IS_CGB_MODE())
        {
            GBC_GPU_CGB_RenderWindowScanline();
        }
        else
        {
            GBC_GPU_DMG_RenderWindowScanline();
        }
    }

    if (GBC_MMU_Memory.IO.SpriteDisplayEnable)
    {
        if (GBC_MMU_IS_CGB_MODE())
        {
            GBC_GPU_CGB_RenderSpriteScanline();
        }
        else
        {
            GBC_GPU_DMG_RenderSpriteScanline();
        }
    }
}

#define GBC_GPU_COMPARE_SCANLINE()                                                                                                         \
{                                                                                                                                          \
    if (GBC_MMU_Memory.IO.Scanline == GBC_MMU_Memory.IO.ScanlineCompare)                                                                   \
    {                                                                                                                                      \
        GBC_MMU_Memory.IO.Coincidence = 1;                                                                                                 \
                                                                                                                                           \
        if (GBC_MMU_Memory.IO.CoincidenceInterrupt)                                                                                        \
        {                                                                                                                                  \
            if (GBC_GPU_StatusInterruptRequestState.RequestFlags == 0)                                                                     \
            {                                                                                                                              \
                GBC_MMU_Memory.IO.InterruptFlags |= GBC_MMU_INTERRUPT_FLAGS_LCD_STAT;                                                      \
            }                                                                                                                              \
                                                                                                                                           \
            GBC_GPU_StatusInterruptRequestState.CoincidenceInterruptRequest = 1;                                                           \
        }                                                                                                                                  \
    }                                                                                                                                      \
    else                                                                                                                                   \
    {                                                                                                                                      \
        GBC_MMU_Memory.IO.Coincidence = 0;                                                                                                 \
        GBC_GPU_StatusInterruptRequestState.CoincidenceInterruptRequest = 0;                                                               \
    }                                                                                                                                      \
}                                                                                                                                          \

bool GBC_GPU_Step(void)
{
    GBC_GPU_ModeTicks += GBC_CPU_StepTicks;

    switch (GBC_GPU_Mode)
    {
        case GBC_GPU_MODE_0_DURING_HBLANK:           // During HBlank period: CPU can access VRAM and OAM
        {
            if (GBC_GPU_ModeTicks >= 204)
            {
                GBC_GPU_ModeTicks -= 204;

                GBC_MMU_Memory.IO.Scanline++;
                GBC_GPU_COMPARE_SCANLINE();

                if (GBC_MMU_HDMAEnabled)             // Can only be enabled in GBC mode
                {
                    if (!GBC_CPU_Halted || (GBC_MMU_Memory.InterruptEnable & GBC_MMU_Memory.IO.InterruptFlags) /* Pending Interrupts */)
                    {
                        GBC_GPU_ModeTicks += GBC_MMU_StartHDMATransfer();
                    }
                }

                RC.CurrFrameBufferStartIndex += 160;
                RC.CurrFrameBufferEndIndex += 160;

                if (GBC_MMU_Memory.IO.Scanline >= 144)
                {
                    GBC_MMU_Memory.IO.InterruptFlags |= GBC_MMU_INTERRUPT_FLAGS_VBLANK;

                    if (GBC_MMU_Memory.IO.VBlankInterrupt)
                    {
                        if (GBC_GPU_StatusInterruptRequestState.RequestFlags == 0)
                        {
                            GBC_MMU_Memory.IO.InterruptFlags |= GBC_MMU_INTERRUPT_FLAGS_LCD_STAT;
                        }

                        GBC_GPU_StatusInterruptRequestState.VBlankInterruptRequest = 1;
                    }

                    GBC_GPU_StatusInterruptRequestState.HBlankInterruptRequest = 0;

                    GBC_GPU_Mode = GBC_MMU_Memory.IO.GPUMode = GBC_GPU_MODE_1_DURING_VBLANK;

#ifdef GBC_GPU_FRAME_RATE_30HZ_MODE
                    if (GBC_GPU_SkipCurrentFrame)
                    {
                        GBC_GPU_SkipCurrentFrame = false;
                    }
                    else
                    {
                        GBC_GPU_SkipCurrentFrame = true;
                        return true;
                    }
#else
                    return true;
#endif
                }
                else
                {
                    if (GBC_MMU_Memory.IO.OAMInterrupt)
                    {
                        if (GBC_GPU_StatusInterruptRequestState.RequestFlags == 0)
                        {
                            GBC_MMU_Memory.IO.InterruptFlags |= GBC_MMU_INTERRUPT_FLAGS_LCD_STAT;
                        }

                        GBC_GPU_StatusInterruptRequestState.OAMInterruptRequest = 1;
                    }

                    GBC_GPU_StatusInterruptRequestState.HBlankInterruptRequest = 0;

                    GBC_GPU_Mode = GBC_MMU_Memory.IO.GPUMode = GBC_GPU_MODE_2_DURING_OAM_READING;
                }
            }
            break;
        }
        case GBC_GPU_MODE_1_DURING_VBLANK:           // During VBlank period: CPU can access VRAM and OAM
        {
            if (GBC_GPU_ModeTicks >= 456)
            {
                GBC_GPU_ModeTicks -= 456;

                if (GBC_MMU_Memory.IO.Scanline >= 153)
                {
                    GBC_MMU_Memory.IO.Scanline = 0;
                    GBC_GPU_COMPARE_SCANLINE();

                    RC.CurrFrameBufferStartIndex = 0;
                    RC.CurrFrameBufferEndIndex = 160;

                    if (GBC_MMU_Memory.IO.OAMInterrupt)
                    {
                        if (GBC_GPU_StatusInterruptRequestState.RequestFlags == 0)
                        {
                            GBC_MMU_Memory.IO.InterruptFlags |= GBC_MMU_INTERRUPT_FLAGS_LCD_STAT;
                        }

                        GBC_GPU_StatusInterruptRequestState.OAMInterruptRequest = 1;
                    }

                    GBC_GPU_StatusInterruptRequestState.VBlankInterruptRequest = 0;

                    GBC_GPU_Mode = GBC_MMU_Memory.IO.GPUMode = GBC_GPU_MODE_2_DURING_OAM_READING;
                }
                else
                {
                    GBC_MMU_Memory.IO.Scanline++;
                    GBC_GPU_COMPARE_SCANLINE();
                }
            }
            break;
        }
        case GBC_GPU_MODE_2_DURING_OAM_READING:      // During reading from OAM: CPU can NOT access OAM
        {
            if (GBC_GPU_ModeTicks >= 80)
            {
                GBC_GPU_ModeTicks -= 80;

                GBC_GPU_StatusInterruptRequestState.OAMInterruptRequest = 0;

                GBC_GPU_Mode = GBC_MMU_Memory.IO.GPUMode = GBC_GPU_MODE_3_DURING_DATA_TRANSFER;
            }
            break;
        }
        case GBC_GPU_MODE_3_DURING_DATA_TRANSFER:    // During reading from OAM and VRAM: CPU can NOT access OAM and VRAM        - GBC mode: Can not access Palette Data (FF69, FF6B) too
        {
            if (GBC_GPU_ModeTicks >= 172)
            {
                GBC_GPU_ModeTicks -= 172;

#ifdef GBC_GPU_FRAME_RATE_30HZ_MODE
                if (!GBC_GPU_SkipCurrentFrame)
                {
                    GBC_GPU_RenderScanline();
                }
#else
                GBC_GPU_RenderScanline();
#endif

                if (GBC_MMU_Memory.IO.HBlankInterrupt)
                {
                    if (GBC_GPU_StatusInterruptRequestState.RequestFlags == 0)
                    {
                        GBC_MMU_Memory.IO.InterruptFlags |= GBC_MMU_INTERRUPT_FLAGS_LCD_STAT;
                    }

                    GBC_GPU_StatusInterruptRequestState.HBlankInterruptRequest = 1;
                }

                GBC_GPU_Mode = GBC_MMU_Memory.IO.GPUMode = GBC_GPU_MODE_0_DURING_HBLANK;
            }
            break;
        }
    }

    return false;
}
