#ifndef GBC_MMU_H
#define GBC_MMU_H

#include "common.h"

typedef enum GBC_MMU_CartridgeType_e
{
    GBC_MMU_CARTRIDGE_TYPE_ROM_ONLY                       = 0x00,
    GBC_MMU_CARTRIDGE_TYPE_MBC1                           = 0x01,
    GBC_MMU_CARTRIDGE_TYPE_MBC1_RAM                       = 0x02,
    GBC_MMU_CARTRIDGE_TYPE_MBC1_RAM_BATTERY               = 0x03,
    GBC_MMU_CARTRIDGE_TYPE_MBC2                           = 0x05,
    GBC_MMU_CARTRIDGE_TYPE_MBC2_BATTERY                   = 0x06,
    GBC_MMU_CARTRIDGE_TYPE_ROM_RAM                        = 0x08,
    GBC_MMU_CARTRIDGE_TYPE_ROM_RAM_BATTERY                = 0x09,
    GBC_MMU_CARTRIDGE_TYPE_MMM01                          = 0x0B,
    GBC_MMU_CARTRIDGE_TYPE_MMM01_RAM                      = 0x0C,
    GBC_MMU_CARTRIDGE_TYPE_MMM01_RAM_BATTERY              = 0x0D,
    GBC_MMU_CARTRIDGE_TYPE_MBC3_TIMER_BATTERY             = 0x0F,
    GBC_MMU_CARTRIDGE_TYPE_MBC3_TIMER_RAM_BATTERY         = 0x10,
    GBC_MMU_CARTRIDGE_TYPE_MBC3                           = 0x11,
    GBC_MMU_CARTRIDGE_TYPE_MBC3_RAM                       = 0x12,
    GBC_MMU_CARTRIDGE_TYPE_MBC3_RAM_BATTERY               = 0x13,
    GBC_MMU_CARTRIDGE_TYPE_MBC5                           = 0x19,
    GBC_MMU_CARTRIDGE_TYPE_MBC5_RAM                       = 0x1A,
    GBC_MMU_CARTRIDGE_TYPE_MBC5_RAM_BATTERY               = 0x1B,
    GBC_MMU_CARTRIDGE_TYPE_MBC5_RUMBLE                    = 0x1C,
    GBC_MMU_CARTRIDGE_TYPE_MBC5_RUMBLE_RAM                = 0x1D,
    GBC_MMU_CARTRIDGE_TYPE_MBC5_RUMBLE_RAM_BATTERY        = 0x1E,
    GBC_MMU_CARTRIDGE_TYPE_MBC6                           = 0x20,
    GBC_MMU_CARTRIDGE_TYPE_MBC7_SENSOR_RUMBLE_RAM_BATTERY = 0x22,
    GBC_MMU_CARTRIDGE_TYPE_POCKET_CAMERA                  = 0xFC,
    GBC_MMU_CARTRIDGE_TYPE_BANDAI_TAMA5                   = 0xFD,
    GBC_MMU_CARTRIDGE_TYPE_HuC3                           = 0xFE,
    GBC_MMU_CARTRIDGE_TYPE_HuC1_RAM_BATTERY               = 0xFF,
}
GBC_MMU_CartridgeType_t;

typedef enum GBC_MMU_MemoryBankController_e
{
    GBC_MMU_MBC_NONE = 0,
    GBC_MMU_MBC1     = 1,
    GBC_MMU_MBC2     = 2,
    GBC_MMU_MBC3     = 3,
    GBC_MMU_MBC5     = 4,
}
GBC_MMU_MemoryBankController_t;

typedef enum GBC_MMU_MBC1Mode_e
{
    GBC_MMU_MBC1_MODE_ROM = 0, // Up to  8KByte RAM, 2MByte ROM
    GBC_MMU_MBC1_MODE_RAM = 1, // Up to 32KByte RAM, 512KByte ROM
}
GBC_MMU_MBC1Mode_t;

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
        uint8_t CartridgeBank0[16384];    // 0000-3FFF: 16kB Cartridge ROM bank 0

        struct
        {
            uint8_t Bootstrap[256];       // 0000-00FF
            uint8_t EntryPoint[4];        // 0100-0103
            uint8_t NintendoLogo[48];     // 0104-0133
            uint8_t Title[11];            // 0134-013E
            uint8_t ManufacturerCode[4];  // 013F-0142
            uint8_t CGBFlag;              // 0143
            uint8_t NewLicenseeCode[2];   // 0144-0145
            uint8_t SGBFlag;              // 0146
            uint8_t CartridgeType;        // 0147
            uint8_t ROMSize;              // 0148
            uint8_t RAMSize;              // 0149
            uint8_t DestinationCode;      // 014A
            uint8_t OldLicenseeCode;      // 014B
            uint8_t MaskROMVersionNumber; // 014C
            uint8_t HeaderChecksum;       // 014D
            uint8_t GlobalChecksum[2];    // 014E-014F
            // Program Code               // 0150-3FFF
        };
    };
    //------CartridgeBankX                   4000-7FFF: 16kB Cartridge ROM bank X
    uint8_t VRAMBank0[8192];              // 8000-9FFF:  8kB Video RAM bank X                 - Switchable only in GBC mode (0-1)
    uint8_t VRAMBank1[8192];
    uint8_t ERAMBank0[8192];              // A000-BFFF:  8kB External Cartridge RAM bank X
    uint8_t ERAMBank1[8192];
    uint8_t ERAMBank2[8192];
    uint8_t ERAMBank3[8192];
    uint8_t WRAMBank0[4096];              // C000-CFFF:  4kB Work RAM Bank 0
    uint8_t WRAMBank1[4096];              // D000-DFFF:  4kB Work RAM Bank X                  - Switchable only in GBC mode (1-7)
    uint8_t WRAMBank2[4096];
    uint8_t WRAMBank3[4096];
    uint8_t WRAMBank4[4096];
    uint8_t WRAMBank5[4096];
    uint8_t WRAMBank6[4096];
    uint8_t WRAMBank7[4096];
    //------ShadowRAM                     // E000-FDFF: 7.5kB Shadow RAM                      - Unused due to the original GBC wiring
    uint8_t OAM[160];                     // FE00-FE9F: 160B Object Attribute Memory
    //------Unused                        // FEA0-FEFF:  96B Unused
    union
    {
        uint8_t IO[128];                  // FF00-FF7F: 128B Memory-mapped I/O

        struct
        {
            uint8_t IO_Unk1[15];
            uint8_t InterruptFlags;       // 0xFF0F
            uint8_t IO_Unk2[48];
            uint8_t GPUControlFlags;      // 0xFF40
            uint8_t IO_Unk3;
            uint8_t GPUScrollY;           // 0xFF42
            uint8_t GPUScrollX;           // 0xFF43
        };
    };
    uint8_t HRAM[127];                    // FF80-FFFE: 127B High RAM
    uint8_t InterruptEnableRegister;      // FFFF:        1B Interrupt enable register
}
GBC_MMU_Memory_t;

bool GBC_MMU_LoadFromCartridge(void);
bool GBC_MMU_LoadFromSDC(char* fileName);
void GBC_MMU_Unload(void);

uint8_t GBC_MMU_ReadByte(uint16_t address);
uint16_t GBC_MMU_ReadShort(uint16_t address);

void GBC_MMU_WriteByte(uint16_t address, uint8_t value);
void GBC_MMU_WriteShort(uint16_t address, uint16_t value);

GBC_MMU_MemoryBankController_t GBC_MMU_GetMemoryBankController(void);

#endif
