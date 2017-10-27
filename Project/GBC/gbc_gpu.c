#include "gbc_gpu.h"
#include "gbc_cpu.h"
#include "gbc_mmu.h"

uint32_t GBC_GPU_ModeTicks = 0;

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

                // ToDo: Write scanline to frame buffer

                GBC_MMU_Memory.GPUStatus = GBC_GPU_MODE_0_DURING_HBLANK;
            }
            break;
        }
    }
}
