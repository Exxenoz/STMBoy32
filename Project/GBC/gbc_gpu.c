#include "gbc_gpu.h"
#include "gbc_cpu.h"
#include "gbc_mmu.h"

uint32_t GBC_GPU_ModeTicks = 0;
GBC_GPU_StatusInterruptRequestState_t GBC_GPU_StatusInterruptRequestState;
uint16_t GBC_GPU_CurrentFrameBufferStartIndex = 0;
uint16_t GBC_GPU_CurrentFrameBufferEndIndex = 160;
GBC_GPU_Color_t GBC_GPU_FrameBuffer[160 * 144];
GBC_GPU_Color_t GBC_GPU_BackgroundPaletteClassic[4] =
{
    0xFFFF, // 0 - White
    0x524A, // 1 - 33% on
    0xA294, // 2 - 66% on
    0x0000, // 3 - Black
};

void GBC_GPU_Initialize(void)
{
    GBC_GPU_ModeTicks = 0;
    GBC_GPU_StatusInterruptRequestState.RequestFlags = 0;
    GBC_GPU_CurrentFrameBufferStartIndex = 0;
    GBC_GPU_CurrentFrameBufferEndIndex = 160;
    // Frame buffer must not be initialized
}

void GBC_GPU_RenderScanline(void)
{
    // Where to render on the frame buffer
    uint16_t frameBufferIndex = GBC_GPU_CurrentFrameBufferStartIndex;

    // When the display is disabled the screen is blank (white)
    if (!GBC_MMU_Memory.DisplayEnable)
    {
        while (frameBufferIndex < GBC_GPU_CurrentFrameBufferEndIndex)
        {
            GBC_GPU_FrameBuffer[frameBufferIndex++] = GBC_GPU_BackgroundPaletteClassic[0];
        }

        return;
    }

    // Which row in the tile map needs to be rendered
    uint16_t screenOffsetY = GBC_MMU_Memory.Scanline + GBC_MMU_Memory.ScrollY;

    // Which tile to start with in the map row
    uint8_t tileX = GBC_MMU_Memory.ScrollX >> 3;

    // Which tile to start with in the map column
    uint8_t tileY = (screenOffsetY & 0xFF) >> 3;

    // Which column of pixels in the tile to start with
    // Numbered from left to right (0-7)
    uint8_t pixelX = GBC_MMU_Memory.ScrollX & 7;

    // Which row of pixels to use in the tiles
    // Numbered from top to bottom (0-7)
    uint8_t pixelY = screenOffsetY & 7;

    // Which start tile to use in tile map #1
    uint16_t tileIndex = (tileY << 5) + tileX;

    // In GBC mode the BGDisplayEnable flag is ignored and sprites are always displayed on top
    if (GBC_MMU_Memory.BGDisplayEnable || (GBC_MMU_Memory.CGBFlag & (GBC_MMU_CGB_FLAG_SUPPORTED | GBC_MMU_CGB_FLAG_ONLY)))
    {
        // Check if tile map #2 is selected
        if (GBC_MMU_Memory.BGTileMapDisplaySelect)
        {
            tileIndex += 1024; // Use tile map #2
        }

        for (; frameBufferIndex < GBC_GPU_CurrentFrameBufferEndIndex; tileIndex++)
        {
            // Read tile index from tile map
            uint16_t tileID = GBC_MMU_Memory.TileMapData[tileIndex];

            // Calculate the real tile ID if tileID is signed due to tile set #0 selection
            if (!GBC_MMU_Memory.BGAndWindowTileSetDisplaySelect && tileID < 128)
            {
                tileID += 256;
            }

            // Each tile is 8x8 pixel in size and uses 2 bytes per row or 2 bits per pixel
            uint16_t tilePixelLine = GBC_MMU_Memory.TileSetData[(tileID << 3) + pixelY];

            // Move first pixel bits to bit 15 (low) and bit 7 (high)
            tilePixelLine <<= pixelX;

            for (; frameBufferIndex < GBC_GPU_CurrentFrameBufferEndIndex && pixelX < 8; pixelX++)
            {
                uint8_t pixel = ((tilePixelLine & 0x80) >> 6) | ((tilePixelLine & 0x8000) >> 15);

                switch (pixel)
                {
                    case 0:
                        GBC_GPU_FrameBuffer[frameBufferIndex++] = GBC_GPU_BackgroundPaletteClassic[GBC_MMU_Memory.BackgroundPaletteColor0];
                        break;
                    case 1:
                        GBC_GPU_FrameBuffer[frameBufferIndex++] = GBC_GPU_BackgroundPaletteClassic[GBC_MMU_Memory.BackgroundPaletteColor1];
                        break;
                    case 2:
                        GBC_GPU_FrameBuffer[frameBufferIndex++] = GBC_GPU_BackgroundPaletteClassic[GBC_MMU_Memory.BackgroundPaletteColor2];
                        break;
                    case 3:
                        GBC_GPU_FrameBuffer[frameBufferIndex++] = GBC_GPU_BackgroundPaletteClassic[GBC_MMU_Memory.BackgroundPaletteColor3];
                        break;
                }

                tilePixelLine <<= 1;
            }

            pixelX = 0;
        }
    }
    // When BGDisplayEnable is false and we are not in GBC mode, the background becomes blank (white).
    else while (frameBufferIndex < GBC_GPU_CurrentFrameBufferEndIndex)
    {
        GBC_GPU_FrameBuffer[frameBufferIndex++] = GBC_GPU_BackgroundPaletteClassic[0];
    }

    // The window becomes visible (if enabled) when positions are set in range WX = 0-166, WY = 0-143
    if (GBC_MMU_Memory.WindowDisplayEnable &&
        //GBC_MMU_Memory.WindowPositionY <= 143 && -- Not needed, because Scanline should be in range 0-143 at this position
        GBC_MMU_Memory.WindowPositionY <= GBC_MMU_Memory.Scanline &&
        GBC_MMU_Memory.WindowPositionXMinus7 <= 166) // X = 7 and Y = 0 locates window at upper left
    {
        // Where to render on the frame buffer
        if (GBC_MMU_Memory.WindowPositionXMinus7 > 7)
        {
            frameBufferIndex = GBC_GPU_CurrentFrameBufferStartIndex + GBC_MMU_Memory.WindowPositionXMinus7 - 7;
        }
        else
        {
            frameBufferIndex = GBC_GPU_CurrentFrameBufferStartIndex;
        }

        // Which row in the tile map needs to be rendered
        screenOffsetY = GBC_MMU_Memory.Scanline - GBC_MMU_Memory.WindowPositionY;

        // Which tile to start with in the map row
        tileX = 0;

        // Which tile to start with in the map column
        tileY = (screenOffsetY & 0xFF) >> 3;

        // Which column of pixels in the tile to start with
        // Numbered from left to right (0-7)
        pixelX = (GBC_MMU_Memory.WindowPositionXMinus7 < 7) ? 7 - GBC_MMU_Memory.WindowPositionXMinus7 : 0;

        // Which row of pixels to use in the tiles
        // Numbered from top to bottom (0-7)
        pixelY = screenOffsetY & 7;

        // Which start tile to use in tile map #1
        tileIndex = (tileY << 5) + tileX;

        // Check if tile map #2 is selected
        if (GBC_MMU_Memory.WindowTileMapDisplaySelect)
        {
            tileIndex += 1024; // Use tile map #2
        }

        for (; frameBufferIndex < GBC_GPU_CurrentFrameBufferEndIndex; tileIndex++)
        {
            // Read tile index from tile map
            uint16_t tileID = GBC_MMU_Memory.TileMapData[tileIndex];

            // Calculate the real tile ID if tileID is signed due to tile set #0 selection
            if (!GBC_MMU_Memory.BGAndWindowTileSetDisplaySelect && tileID < 128)
            {
                tileID += 256;
            }

            // Each tile is 8x8 pixel in size and uses 2 bytes per row or 2 bits per pixel
            uint16_t tilePixelLine = GBC_MMU_Memory.TileSetData[(tileID << 3) + pixelY];

            // Move first pixel bits to bit 15 (low) and bit 7 (high)
            tilePixelLine <<= pixelX;

            for (; frameBufferIndex < GBC_GPU_CurrentFrameBufferEndIndex && pixelX < 8; pixelX++)
            {
                uint8_t pixel = ((tilePixelLine & 0x80) >> 6) | ((tilePixelLine & 0x8000) >> 15);

                switch (pixel)
                {
                    case 0:
                        GBC_GPU_FrameBuffer[frameBufferIndex++] = GBC_GPU_BackgroundPaletteClassic[GBC_MMU_Memory.BackgroundPaletteColor0];
                        break;
                    case 1:
                        GBC_GPU_FrameBuffer[frameBufferIndex++] = GBC_GPU_BackgroundPaletteClassic[GBC_MMU_Memory.BackgroundPaletteColor1];
                        break;
                    case 2:
                        GBC_GPU_FrameBuffer[frameBufferIndex++] = GBC_GPU_BackgroundPaletteClassic[GBC_MMU_Memory.BackgroundPaletteColor2];
                        break;
                    case 3:
                        GBC_GPU_FrameBuffer[frameBufferIndex++] = GBC_GPU_BackgroundPaletteClassic[GBC_MMU_Memory.BackgroundPaletteColor3];
                        break;
                }

                tilePixelLine <<= 1;
            }

            pixelX = 0;
        }
    }
}

void GBC_GPU_CompareScanline()
{
    if (GBC_MMU_Memory.Scanline == GBC_MMU_Memory.ScanlineCompare)
    {
        GBC_MMU_Memory.Coincidence = 1;

        if (GBC_MMU_Memory.CoincidenceInterrupt)
        {
            if (GBC_GPU_StatusInterruptRequestState.RequestFlags == 0)
            {
                GBC_MMU_Memory.InterruptFlags |= GBC_MMU_INTERRUPT_FLAGS_LCD_STAT;
            }

            GBC_GPU_StatusInterruptRequestState.CoincidenceInterruptRequest = 1;
        }
    }
    else
    {
        GBC_MMU_Memory.Coincidence = 0;
        GBC_GPU_StatusInterruptRequestState.CoincidenceInterruptRequest = 0;
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
                GBC_GPU_CompareScanline();

                GBC_GPU_CurrentFrameBufferStartIndex += 160;
                GBC_GPU_CurrentFrameBufferEndIndex += 160;

                if (GBC_MMU_Memory.Scanline >= 144)
                {
                    GBC_MMU_Memory.InterruptFlags |= GBC_MMU_INTERRUPT_FLAGS_VBLANK;

                    if (GBC_MMU_Memory.VBlankInterrupt)
                    {
                        if (GBC_GPU_StatusInterruptRequestState.RequestFlags == 0)
                        {
                            GBC_MMU_Memory.InterruptFlags |= GBC_MMU_INTERRUPT_FLAGS_LCD_STAT;
                        }

                        GBC_GPU_StatusInterruptRequestState.VBlankInterruptRequest = 1;
                    }

                    GBC_GPU_StatusInterruptRequestState.HBlankInterruptRequest = 0;

                    GBC_MMU_Memory.GPUStatus = GBC_GPU_MODE_1_DURING_VBLANK;
                }
                else
                {
                    if (GBC_MMU_Memory.OAMInterrupt)
                    {
                        if (GBC_GPU_StatusInterruptRequestState.RequestFlags == 0)
                        {
                            GBC_MMU_Memory.InterruptFlags |= GBC_MMU_INTERRUPT_FLAGS_LCD_STAT;
                        }

                        GBC_GPU_StatusInterruptRequestState.OAMInterruptRequest = 1;
                    }

                    GBC_GPU_StatusInterruptRequestState.HBlankInterruptRequest = 0;

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

                if (GBC_MMU_Memory.Scanline >= 153)
                {
                    GBC_MMU_Memory.Scanline = 0;
                    GBC_GPU_CompareScanline();

                    GBC_GPU_CurrentFrameBufferStartIndex = 0;
                    GBC_GPU_CurrentFrameBufferEndIndex = 160;

                    if (GBC_MMU_Memory.OAMInterrupt)
                    {
                        if (GBC_GPU_StatusInterruptRequestState.RequestFlags == 0)
                        {
                            GBC_MMU_Memory.InterruptFlags |= GBC_MMU_INTERRUPT_FLAGS_LCD_STAT;
                        }

                        GBC_GPU_StatusInterruptRequestState.OAMInterruptRequest = 1;
                    }

                    GBC_GPU_StatusInterruptRequestState.VBlankInterruptRequest = 0;

                    GBC_MMU_Memory.GPUStatus = GBC_GPU_MODE_2_DURING_OAM_READING;
                }
                else
                {
                    GBC_MMU_Memory.Scanline++;
                    GBC_GPU_CompareScanline();
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

                if (GBC_MMU_Memory.HBlankInterrupt)
                {
                    if (GBC_GPU_StatusInterruptRequestState.RequestFlags == 0)
                    {
                        GBC_MMU_Memory.InterruptFlags |= GBC_MMU_INTERRUPT_FLAGS_LCD_STAT;
                    }

                    GBC_GPU_StatusInterruptRequestState.HBlankInterruptRequest = 1;
                }

                GBC_MMU_Memory.GPUStatus = GBC_GPU_MODE_0_DURING_HBLANK;
            }
            break;
        }
    }
}
