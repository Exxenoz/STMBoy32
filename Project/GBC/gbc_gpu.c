#include "gbc_gpu.h"
#include "gbc_cpu.h"
#include "gbc_mmu.h"

uint32_t GBC_GPU_ModeTicks = 0;
GBC_GPU_Color_t GBC_GPU_FrameBuffer[160 * 144];
GBC_GPU_Color_t GBC_CPU_BackgroundPaletteClassic[4] =
{
    0xFFFF, // 0 - White
    0x524A, // 1 - 33% on
    0xA294, // 2 - 66% on
    0x0000, // 3 - Black
};

void GBC_GPU_RenderScanline(void)
{
    // Which tile to start with in the map row
    uint8_t tileX = GBC_MMU_Memory.ScrollX >> 3;

    // Which tile to start with in the map column
    uint8_t tileY = ((GBC_MMU_Memory.Scanline + GBC_MMU_Memory.ScrollY) & 0xFF) >> 3;

    // Which column of pixels in the tile to start with
    // Numbered from left to right (0-7)
    uint8_t pixelX = GBC_MMU_Memory.ScrollX & 7;

    // Which row of pixels to use in the tiles
    // Numbered from top to bottom (0-7)
    const uint8_t pixelY = (GBC_MMU_Memory.Scanline + GBC_MMU_Memory.ScrollY) & 7;

    // Which start tile to use in tile map #1
    uint16_t tileIndex = (tileY << 5) + tileX;

    // Check if tile map #2 is selected
    if (GBC_MMU_Memory.BGTileMapDisplaySelect)
    {
        tileIndex += 1024; // Use tile map #2
    }

    // Where to render on the frame buffer
    uint16_t frameBufferIndex = GBC_MMU_Memory.Scanline * 160;

    for (long i = 0; i < 160; i++, tileIndex++)
    {
        // Read tile index from background map
        uint16_t tileID = GBC_MMU_Memory.TileMapData[tileIndex];

        // Calculate the real tile ID if tileID is signed due to tile set #0 selection
        if (!GBC_MMU_Memory.BGTileSetDisplaySelect && tileID < 128)
        {
            tileID += 256;
        }

        // Each tile is 8x8 pixel in size and uses 2 bytes per row or 2 bits per pixel
        uint16_t tilePixelLine = GBC_MMU_Memory.TileSetData[(tileID << 3) + pixelY];

        // Move first pixel bits to bit 15 (low) and bit 7 (high)
        tilePixelLine <<= pixelX;

        for (; i < 160 && pixelX < 8; i++, pixelX++)
        {
            uint8_t pixelColor = ((tilePixelLine & 0x80) >> 6) | ((tilePixelLine & 0x8000) >> 15);

            // ToDo: Use BackgroundPalette
            GBC_GPU_FrameBuffer[frameBufferIndex++] = GBC_CPU_BackgroundPaletteClassic[pixelColor];

            tilePixelLine <<= 1;
        }

        pixelX = 0;
    }
}

void GBC_GPU_Step(void)
{
    GBC_GPU_ModeTicks += GBC_CPU_StepTicks;

    switch (GBC_MMU_Memory.GPUMode)
    {
        case GBC_GPU_MODE_0_DURING_HBLANK:           // During HBlank period: CPU can access VRAM and OAM
        {
            if (GBC_GPU_ModeTicks >= 204)
            {
                GBC_GPU_ModeTicks -= 204;

                GBC_MMU_Memory.Scanline++;

                if (GBC_MMU_Memory.Scanline >= 144)
                {
                    // ToDo: Send frame buffer to screen

                    GBC_MMU_Memory.InterruptFlags |= GBC_MMU_INTERRUPT_FLAGS_VBLANK;

                    GBC_MMU_Memory.GPUStatus = GBC_GPU_MODE_1_DURING_VBLANK;
                }
                else
                {
                    GBC_MMU_Memory.GPUStatus = GBC_GPU_MODE_2_DURING_OAM_READING;
                }
            }
            break;
        }
        case GBC_GPU_MODE_1_DURING_VBLANK:           // During VBlank period: CPU can access VRAM and OAM
        {
            if (GBC_GPU_ModeTicks >= 456)
            {
                GBC_GPU_ModeTicks -= 456;

                GBC_MMU_Memory.Scanline++;

                if (GBC_MMU_Memory.Scanline >= 154)
                {
                    GBC_MMU_Memory.Scanline = 0;

                    GBC_MMU_Memory.GPUStatus = GBC_GPU_MODE_2_DURING_OAM_READING;
                }
            }
            break;
        }
        case GBC_GPU_MODE_2_DURING_OAM_READING:      // During reading from OAM: CPU can NOT access OAM
        {
            if (GBC_GPU_ModeTicks >= 80)
            {
                GBC_GPU_ModeTicks -= 80;

                GBC_MMU_Memory.GPUStatus = GBC_GPU_MODE_3_DURING_DATA_TRANSFER;
            }
            break;
        }
        case GBC_GPU_MODE_3_DURING_DATA_TRANSFER:    // During reading from OAM and VRAM: CPU can NOT access OAM and VRAM        - GBC mode: Can not access Palette Data (FF69, FF6B) too
        {
            if (GBC_GPU_ModeTicks >= 172)
            {
                GBC_GPU_ModeTicks -= 172;

                GBC_GPU_RenderScanline();

                GBC_MMU_Memory.GPUStatus = GBC_GPU_MODE_0_DURING_HBLANK;
            }
            break;
        }
    }
}
