#ifndef GBC_MMU_H
#define GBC_MMU_H

#include "common.h"

enum GBC_MMU_InterruptFlags_e
{
    GBC_MMU_INTERRUPT_FLAGS_NONE     =  0,
    GBC_MMU_INTERRUPT_FLAGS_VBLANK   =  1,
    GBC_MMU_INTERRUPT_FLAGS_LCD_STAT =  2,
    GBC_MMU_INTERRUPT_FLAGS_TIMER    =  4,
    GBC_MMU_INTERRUPT_FLAGS_SERIAL   =  8,
    GBC_MMU_INTERRUPT_FLAGS_JOYPAD   = 16,
};

enum GBC_MMU_MemoryLocations_e
{
    GBC_MMU_MEMLOC_INTERRUPT_FLAGS  = 0xFF0F,
    GBC_MMU_MEMLOC_GPU_CONTROL      = 0xFF40,
    GBC_MMU_MEMLOC_GPU_SCROLL_Y     = 0xFF42,
    GBC_MMU_MEMLOC_GPU_SCROLL_X     = 0xFF43,
    GBC_MMU_MEMLOC_GPU_SCANLINE     = 0xFF44,
    GBC_MMU_MEMLOC_GPU_BG_PALETTE   = 0xFF47,
    GBC_MMU_MEMLOC_INTERRUPT_ENABLE = 0xFFFF,
};

typedef struct GBC_MMU_Memory_s
{
    union
    {
        struct
        {
            uint8_t CartridgeBank0[16384];   // 0000-3FFF: 16kB Cartridge ROM bank 0
            uint8_t CartridgeBankX[16384];   // 4000-7FFF: 16kB Cartridge ROM bank X
            uint8_t VRAMBankX[8192];         // 8000-9FFF:  8kB Video RAM bank X                 - Switchable only in GBC mode (0-1)
            uint8_t ERAMBankX[8192];         // A000-BFFF:  8kB External Cartridge RAM bank X
            uint8_t WRAMBank0[4096];         // C000-CFFF:  4kB Work RAM Bank 0
            uint8_t WRAMBankX[4096];         // D000-DFFF:  4kB Work RAM Bank X                  - Switchable only in GBC mode (1-7)
            uint8_t ShadowRAM[7680];         // E000-FDFF: ~7kB Shadow RAM                       - Unused due to the original GBC wiring
            uint8_t OAM[160];                // FE00-FE9F: 160B Object Attribute Memory
            uint8_t Unused[96];              // FEA0-FEFF:  96B Unused
            uint8_t IO[128];                 // FF00-FF7F: 128B Memory-mapped I/O
            uint8_t HRAM[127];               // FF80-FFFE: 127B High RAM
            uint8_t InterruptEnableRegister; // FFFF:        1B Interrupt enable register
        };

        struct
        {
            uint8_t NoName1[65295];
            uint8_t InterruptFlags;          // 0xFF0F
            uint8_t NoName2[48];
            uint8_t GPUControlFlags;         // 0xFF40
            uint8_t NoName3;
            uint8_t GPUScrollY;              // 0xFF42
            uint8_t GPUScrollX;              // 0xFF43
        };

        uint8_t Data[65536];                 // 65kB GBC Memory
    };
}
GBC_MMU_Memory_t;

uint8_t GBC_MMU_ReadByte(uint16_t address);
uint16_t GBC_MMU_ReadShort(uint16_t address);

void GBC_MMU_WriteByte(uint16_t address, uint8_t value);
void GBC_MMU_WriteShort(uint16_t address, uint16_t value);

bool GBC_MMU_LoadROM(char* fileName);

#endif
