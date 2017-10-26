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
    GBC_MMU_MBC_UNKNOWN = 0,
    GBC_MMU_MBC_NONE    = 1,
    GBC_MMU_MBC1        = 2,
    GBC_MMU_MBC2        = 3,
    GBC_MMU_MBC3        = 4,
    GBC_MMU_MBC5        = 5,
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
    uint8_t ERAMBank0[8192];              // A000-BFFF:  8kB External Cartridge RAM bank X, up to 128kB (but due to exhausted ressources for now only 32kB)
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
            uint8_t Joypad;                  // 0xFF00
            uint8_t SerialTransferData;      // 0xFF01
            uint8_t SerialTransferControl;   // 0xFF02
            uint8_t IO_Unk13;
            uint8_t DividerRegister;         // 0xFF04
            uint8_t TimerCounter;            // 0xFF05
            uint8_t TimerModulo;             // 0xFF06
            uint8_t TimerControl;            // 0xFF07
            uint8_t IO_Unk1[7];
            uint8_t InterruptFlags;          // 0xFF0F
            uint8_t IO_Unk2[48];
            uint8_t GPUControlFlags;         // 0xFF40
            uint8_t GPUStatus;               // 0xFF41
            uint8_t ScrollY;                 // 0xFF42
            uint8_t ScrollX;                 // 0xFF43
            uint8_t Scanline;                // 0xFF44
            uint8_t ScanlineCompare;         // 0xFF45
            uint8_t DMATransferStartAddress; // 0xFF46
            uint8_t GPUBackgroundPalette;    // 0xFF47                                           - Non GBC mode only
            uint8_t ObjectPaletteData0;      // 0xFF48                                           - Non GBC mode only
            uint8_t ObjectPaletteData1;      // 0xFF49                                           - Non GBC mode only
            uint8_t WindowYPosition;         // 0xFF4A
            uint8_t WindowXPositionMinus7;   // 0xFF4B
            uint8_t IO_Unk5;
            uint8_t PrepareSpeedSwitch;      // 0xFF4D                                           - Only in GBC mode
            uint8_t IO_Unk11;
            uint8_t VRAMBankID;              // 0xFF4F                                           - Only in GBC mode
            uint8_t IO_Unk8;
            uint8_t NewDMASourceHigh;        // 0xFF51                                           - Only in GBC mode
            uint8_t NewDMASourceLow;         // 0xFF52                                           - Only in GBC mode
            uint8_t NewDMADestinationHigh;   // 0xFF53                                           - Only in GBC mode
            uint8_t NewDMADestinationLow;    // 0xFF54                                           - Only in GBC mode
            uint8_t NewDMALengthModeStart;   // 0xFF55                                           - Only in GBC mode
            uint8_t InfraredPort;            // 0xFF56                                           - Only in GBC mode
            uint8_t IO_Unk9[17];
            uint8_t BackgroundPaletteIndex;  // 0xFF68                                           - Only in GBC mode
            uint8_t BackgroundPaletteData;   // 0xFF69                                           - Only in GBC mode
            uint8_t SpritePaletteIndex;      // 0xFF6A                                           - Only in GBC mode
            uint8_t SpritePaletteData;       // 0xFF6B                                           - Only in GBC mode
            uint8_t IO_Unk12[4];
            uint8_t WRAMBankID;              // 0xFF70                                           - Only in GBC mode
            uint8_t IO_Unk10[15];
        };
    };
    uint8_t HRAM[127];                    // FF80-FFFE: 127B High RAM
    uint8_t InterruptEnableRegister;      // FFFF:        1B Interrupt enable register
}
GBC_MMU_Memory_t;

typedef union GBC_MMU_RTC_Register_s
{
    uint8_t Data[5];

    struct
    {
        uint8_t S;  // Seconds 0-59 (0-3Bh)
        uint8_t M;  // Minutes 0-59 (0-3Bh)
        uint8_t H;  // Hours 0-23 (0-17h)
        uint8_t DL; // Lower 8 bits of Day Counter (0-FFh)
        uint8_t DH; // Upper 1 bit of Day Counter, Carry Bit, Halt Flag
             // Bit 0  Most significant bit of Day Counter (Bit 8)
             // Bit 6  Halt (0=Active, 1=Stop Timer)
             // Bit 7  Day Counter Carry Bit (1=Counter Overflow)
    };
}
GBC_MMU_RTC_Register_t;

bool GBC_MMU_LoadFromCartridge(void);
bool GBC_MMU_LoadFromSDC(char* fileName);
void GBC_MMU_Unload(void);

uint8_t GBC_MMU_ReadByte(uint16_t address);
uint16_t GBC_MMU_ReadShort(uint16_t address);

void GBC_MMU_WriteByte(uint16_t address, uint8_t value);
void GBC_MMU_WriteShort(uint16_t address, uint16_t value);

GBC_MMU_MemoryBankController_t GBC_MMU_GetMemoryBankController(void);

#endif
