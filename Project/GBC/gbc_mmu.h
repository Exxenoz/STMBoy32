#ifndef GBC_MMU_H
#define GBC_MMU_H

#include "common.h"
#include "cmod.h"
#include "gbc.h"
#include "gbc_tim.h"
#include "input.h"

enum GBC_MMU_CGBFlag_e
{
    GBC_MMU_CGB_FLAG_SUPPORTED = 0x80,
    GBC_MMU_CGB_FLAG_ONLY      = 0xC0,
};

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

enum GBC_MMU_CurrentSpeed_e
{
    GBC_MMU_CURRENT_SPEED_SINGLE = 0,
    GBC_MMU_CURRENT_SPEED_DOUBLE = 1,
};

// Thanks to Imran Nazar @ http://imrannazar.com and Marat Fayzullin, Pascal Felber, Paul Robson and Martin Korth for the Pan Docs!

#pragma pack(1)
typedef struct GBC_MMU_Memory_s
{
    #pragma pack(1)
    union
    {
        uint8_t CartridgeBank0[16384];    // 0000-3FFF: 16kB Cartridge ROM bank 0

        #pragma pack(1)
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
    uint8_t CartridgeBankX[16384];        // 4000-7FFF: 16kB Cartridge ROM bank X
    #pragma pack(1)
    union
    {
        uint8_t VRAMBank0[8192];          // 8000-9FFF:  8kB Video RAM bank X                 - Switchable only in GBC mode (0-1)

        #pragma pack(1)
        struct
        {
            uint16_t TileSetData[3072];   // 8000-97FF: Tileset Data
            uint8_t TileMapData[2048];    // 9800-9FFF: Tilemap Data
        };
    };
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
    #pragma pack(1)
    union
    {
        uint8_t OAM[160];                 // FE00-FE9F: 160B Object Attribute Memory

        #pragma pack(1)
        struct SpriteAttributes_s
        {
            uint8_t PositionY;       // Vertical position on the screen minus 16. Offscreen values Y = 0 or Y >= 160 hides the sprite.
            uint8_t PositionX;       // Horizontal position on the screen minus 8.
            uint8_t TileID;          // Unsigned tile ID selects tile from memory at 8000h-8FFFh.      - GBC mode: Tile can be selected from VRAM Bank 0 or 1
            #pragma pack(1)
            union
            {
                uint8_t AttributeFlags;

                #pragma pack(1)
                struct
                {
                    unsigned int PaletteNumber        : 3; // OBP0-7                                                    - Only in GBC mode
                    unsigned int TileVRAMBank         : 1; // 0 = VRAM Bank 0, 1 = VRAM Bank 1                          - Only in GBC mode
                    unsigned int PaletteNumberClassic : 1; // 0 = OBP0, 1 = OBP1                                        - Non GBC mode only
                    unsigned int FlipX                : 1; // 0 = Normal, 1 = Vertically mirrored
                    unsigned int FlipY                : 1; // 0 = Normal, 1 = Horizontally mirrored
                    unsigned int BGRenderPriority     : 1; // 0 = Object above BG, 1 = Object behind BG color 1-3, BG color 0 is always behind object
                };
            };
        } SpriteAttributes[40];
    };
    //------Unused                        // FEA0-FEFF:  96B Unused
    #pragma pack(1)
    union
    {
        uint8_t IO[128];                  // FF00-FF7F: 128B Memory-mapped I/O

        #pragma pack(1)
        struct
        {
            #pragma pack(1)
            union
            {
                uint8_t Joypad;              // 0xFF00

                #pragma pack(1)
                struct
                {
                    unsigned int JoypadInputRightOrButtonA : 1; // (0 = Pressed) (ReadOnly)
                    unsigned int JoypadInputLeftOrButtonB  : 1; // (0 = Pressed) (ReadOnly)
                    unsigned int JoypadInputUpOrSelect     : 1; // (0 = Pressed) (ReadOnly)
                    unsigned int JoypadInputDownOrStart    : 1; // (0 = Pressed) (ReadOnly)
                    unsigned int JoypadInputSelectFade     : 1; // (0 = Selected)
                    unsigned int JoypadInputSelectButtons  : 1; // (0 = Selected)
                    unsigned int                           : 2;
                };
            };
            uint8_t SerialTransferData;      // 0xFF01
            uint8_t SerialTransferControl;   // 0xFF02
            uint8_t IO_Unk13;
            uint8_t TimerDivider;            // 0xFF04  Counts up at a fixed 16384Hz; reset to 0 whenever written to
            uint8_t TimerCounter;            // 0xFF05  Counts up at the specified rate; Triggers INT 0x50 when going 255->0
            uint8_t TimerModulo;             // 0xFF06  When Counter overflows to 0, it is reset to start at TimerModulo
            #pragma pack(1)
            union
            {
                uint8_t TimerControl;        // 0xFF07

                #pragma pack(1)
                struct
                {
                    unsigned int TimerSpeed   : 2; // 00: 4096Hz, 01: 262144Hz, 10: 65536Hz, 11: 16384Hz
                    unsigned int TimerRunning : 1; // 1 to run timer, 0 to stop
                    unsigned int              : 5; // Unused
                };
            };
            uint8_t IO_Unk1[7];
            uint8_t InterruptFlags;          // 0xFF0F
            #pragma pack(1)
            union
            {
                uint8_t Channel1Sweep;       // 0xFF10

                #pragma pack(1)
                struct
                {
                    unsigned int Channel1SweepShift : 3; // Number of sweep shift (n: 0-7)
                    unsigned int Channel1SweepType  : 1; // 0: Addition (frequency increases), 1: Subtraction (frequency decreases)
                    unsigned int Channel1SweepTime  : 3; /* 000: Sweep off - no freq change
                                                            001:  7.8 ms (1/128Hz)
                                                            010: 15.6 ms (2/128Hz)
                                                            011: 23.4 ms (3/128Hz)
                                                            100: 31.3 ms (4/128Hz)
                                                            101: 39.1 ms (5/128Hz)
                                                            110: 46.9 ms (6/128Hz)
                                                            111: 54.7 ms (7/128Hz) */
                    unsigned int                    : 1;
                };
            };
            #pragma pack(1)
            union
            {
                uint8_t Channel1SoundLengthAndWavePatternDuty; // 0xFF11

                #pragma pack(1)
                struct
                {
                    unsigned int Channel1SoundLengthData : 6; // Sound Length = (64-t1)*(1/256) seconds. The length value is used only if Bit 6 in 0xFF14 is set.
                    unsigned int Channel1WavePatternDuty : 2; /* 00: 12.5% ( _-------_-------_------- )
                                                                 01: 25%   ( __------__------__------ )
                                                                 10: 50%   ( ____----____----____---- ) (normal)
                                                                 11: 75%   ( ______--______--______-- ) */
                };
            };
            #pragma pack(1)
            union
            {
                uint8_t Channel1VolumeEnvelope; // 0xFF12

                #pragma pack(1)
                struct
                {
                    unsigned int Channel1EnvelopeSweepNumber   : 3; // Number of envelope sweep (n: 0-7) (If zero, stop envelope operation.) Length of 1 step = n*(1/64) seconds
                    unsigned int Channel1EnvelopeDirection     : 1; // Envelope Direction (0 = Decrease, 1 = Increase)
                    unsigned int Channel1InitialEnvelopeVolume : 4; // Initial volume of envelope (0-0Fh) (0 = No sound)
                };
            };
            #pragma pack(1)
            union
            {
                uint16_t Channel1FrequencyData; // 0xFF13, 0xFF14

                #pragma pack(1)
                struct
                {
                    unsigned int Channel1Frequency        : 11; // Frequency X -> Frequency = 131072/(2048-X) Hz
                    unsigned int                          :  3;
                    unsigned int Channel1CounterSelection :  1; // (1 = Stop output when length in 0xFF11 expires)
                    unsigned int Channel1InitialRestart   :  1; // Initial (1 = Restart Sound)
                };
            };
            uint8_t IO_Unk14; // 0xFF15
            #pragma pack(1)
            union
            {
                uint8_t Channel2SoundLengthAndWavePatternDuty; // 0xFF16

                #pragma pack(1)
                struct
                {
                    unsigned int Channel2SoundLengthData : 6; // Sound Length = (64-t1)*(1/256) seconds. The length value is used only if Bit 6 in 0xFF19 is set.
                    unsigned int Channel2WavePatternDuty : 2; /* 00: 12.5% ( _-------_-------_------- )
                                                                 01: 25%   ( __------__------__------ )
                                                                 10: 50%   ( ____----____----____---- ) (normal)
                                                                 11: 75%   ( ______--______--______-- ) */
                };
            };
            #pragma pack(1)
            union
            {
                uint8_t Channel2VolumeEnvelope;     // 0xFF17

                #pragma pack(1)
                struct
                {
                    unsigned int Channel2EnvelopeSweepNumber   : 3; // Number of envelope sweep (n: 0-7) (If zero, stop envelope operation.) Length of 1 step = n*(1/64) seconds
                    unsigned int Channel2EnvelopeDirection     : 1; // Envelope Direction (0 = Decrease, 1 = Increase)
                    unsigned int Channel2InitialEnvelopeVolume : 4; // Initial volume of envelope (0-0Fh) (0 = No sound)
                };
            };
            #pragma pack(1)
            union
            {
                uint16_t Channel2FrequencyData;     // 0xFF18, 0xFF19

                #pragma pack(1)
                struct
                {
                    unsigned int Channel2Frequency        : 11; // Frequency X -> Frequency = 131072/(2048-X) Hz
                    unsigned int                          :  3;
                    unsigned int Channel2CounterSelection :  1; // (1 = Stop output when length in 0xFF16 expires)
                    unsigned int Channel2InitialRestart   :  1; // Initial (1 = Restart Sound)
                };
            };
            #pragma pack(1)
            union
            {
                uint8_t Channel3SoundOnOffData;     // 0xFF1A

                #pragma pack(1)
                struct
                {
                    unsigned int                      : 7;
                    unsigned int Channel3SoundEnabled : 1; // Sound Channel 3 Off  (0 = Stop, 1 = Playback)
                };
            };
            uint8_t Channel3SoundLength;    // 0xFF1B          Sound length (t1: 0 - 255), Sound Length = (256-t1)*(1/256) seconds. This value is used only if Bit 6 in 0xFF1E is set.
            #pragma pack(1)
            union
            {
                uint8_t Channel3SelectOutputLevelData; // 0xFF1C

                #pragma pack(1)
                struct
                {
                    unsigned int                           : 5;
                    unsigned int Channel3SelectOutputLevel : 2; /* Possible Output levels are:
                                                                   0: Mute (No sound)
                                                                   1: 100% Volume (Produce Wave Pattern RAM Data as it is)
                                                                   2:  50% Volume (Produce Wave Pattern RAM data shifted once to the right)
                                                                   3:  25% Volume (Produce Wave Pattern RAM data shifted twice to the right) */
                    unsigned int                           : 1;
                };
            };
            #pragma pack(1)
            union
            {
                uint16_t Channel3FrequencyData; // 0xFF1D, 0xFF1E

                #pragma pack(1)
                struct
                {
                    unsigned int Channel3Frequency        : 11; // Frequency X -> Frequency = 4194304/(64*(2048-x)) Hz = 65536/(2048-x) Hz
                    unsigned int                          :  3;
                    unsigned int Channel3CounterSelection :  1; // (1 = Stop output when length in 0xFF1B expires)
                    unsigned int Channel3InitialRestart   :  1; // Initial (1 = Restart Sound)
                };
            };
            uint8_t IO_Unk16;                   // 0xFF1F
            uint8_t Channel4SoundLengthData;    // 0xFF20          Sound Length = (64-t1)*(1/256) seconds. The Length value is used only if Bit 6 in 0xFF23 is set.
            #pragma pack(1)
            union
            {
                uint8_t Channel4VolumeEnvelope; // 0xFF21

                #pragma pack(1)
                struct
                {
                    unsigned int Channel4EnvelopeSweepNumber   : 3; // Number of envelope sweep (n: 0-7) (If zero, stop envelope operation.) Length of 1 step = n*(1/64) seconds
                    unsigned int Channel4EnvelopeDirection     : 1; // Envelope Direction (0 = Decrease, 1 = Increase)
                    unsigned int Channel4InitialEnvelopeVolume : 4; // Initial volume of envelope (0-0Fh) (0 = No sound)
                };
            };
            #pragma pack(1)
            union
            {
                uint8_t Channel4PolynomialCounter; // 0xFF22                Frequency = 524288 Hz / r / 2^(s+1) ; For r = 0 assume r = 0.5 instead

                #pragma pack(1)
                struct
                {
                    unsigned int Channel4PolynomialCounterFreqDivRatio   : 3; // Dividing Ratio of Frequencies (r)
                    unsigned int Channel4PolynomialCounterStepWidth      : 1; // Counter Step/Width (0 = 15 bits, 1 = 7 bits). When set, the output will become more regular, and some frequencies will sound more like Tone than Noise.
                    unsigned int Channel4PolynomialCounterShiftClockFreq : 4; // Shift Clock Frequency (s)
                };
            };
            #pragma pack(1)
            union
            {
                uint8_t Channel4InitialCounterSelectionAndInitialRestart; // 0xFF23

                #pragma pack(1)
                struct
                {
                    unsigned int                          : 6;
                    unsigned int Channel4CounterSelection : 1; // (1 = Stop output when length in 0xFF20 expires)
                    unsigned int Channel4InitialRestart   : 1; // Initial (1 = Restart Sound)
                };
            };
            #pragma pack(1)
            union
            {
                uint8_t ChannelControlTerminal; // 0xFF24

                #pragma pack(1)
                struct
                {
                    unsigned int ChannelControlOutputLevelSO1 : 3; // SO1 output level (volume)  (0-7)
                    unsigned int ChannelControlOutputVinToSO1 : 1; // Output Vin to SO1 terminal (1 = Enable)
                    unsigned int ChannelControlOutputLevelSO2 : 3; // SO2 output level (volume)  (0-7)
                    unsigned int ChannelControlOutputVinToSO2 : 1; // Output Vin to SO2 terminal (1 = Enable)
                };
            };
            #pragma pack(1)
            union
            {
                uint8_t SoundOutputTerminal; // 0xFF25

                #pragma pack(1)
                struct
                {
                    unsigned int SoundOutputChannel1ToSO1 : 1; // Output sound 1 to SO1 terminal
                    unsigned int SoundOutputChannel2ToSO1 : 1; // Output sound 2 to SO1 terminal
                    unsigned int SoundOutputChannel3ToSO1 : 1; // Output sound 3 to SO1 terminal
                    unsigned int SoundOutputChannel4ToSO1 : 1; // Output sound 4 to SO1 terminal
                    unsigned int SoundOutputChannel1ToSO2 : 1; // Output sound 1 to SO2 terminal
                    unsigned int SoundOutputChannel2ToSO2 : 1; // Output sound 2 to SO2 terminal
                    unsigned int SoundOutputChannel3ToSO2 : 1; // Output sound 3 to SO2 terminal
                    unsigned int SoundOutputChannel4ToSO2 : 1; // Output sound 4 to SO2 terminal
                };
            };
            #pragma pack(1)
            union
            {
                uint8_t ChannelSoundTerminal; // 0xFF26

                #pragma pack(1)
                struct
                {
                    unsigned int ChannelSound1Enabled : 1; // Sound 1 ON flag
                    unsigned int ChannelSound2Enabled : 1; // Sound 2 ON flag
                    unsigned int ChannelSound3Enabled : 1; // Sound 3 ON flag
                    unsigned int ChannelSound4Enabled : 1; // Sound 4 ON flag
                    unsigned int                      : 3;
                    unsigned int ChannelSoundsEnabled : 1; // All sound on/off  (0: stop all sound circuits)
                };
            };
            uint8_t IO_Unk15[9];
            uint8_t Channel3WavePatternRAM[16]; // 0xFF30-0xFF3F
            #pragma pack(1)
            union
            {
                uint8_t GPUControlFlags;     // 0xFF40

                #pragma pack(1)
                struct
                {
                    unsigned int BGDisplayEnable                 : 1; // (0 = Off, 1 = On)                - GBC mode: When cleared, the sprites will be always displayed on top of background and window
                    unsigned int SpriteDisplayEnable             : 1; // (0 = Off, 1 = On)
                    unsigned int SpriteSize                      : 1; // (0 = 8x8, 1 = 8x16)
                    unsigned int BGTileMapDisplaySelect          : 1; // (0 = 9800-9BFF, 1 = 9C00-9FFF)
                    unsigned int BGAndWindowTileSetDisplaySelect : 1; // (0 = 8800-97FF, 1 = 8000-8FFF)
                    unsigned int WindowDisplayEnable             : 1; // (0 = Off, 1 = On)
                    unsigned int WindowTileMapDisplaySelect      : 1; // (0 = 9800-9BFF, 1 = 9C00-9FFF)
                    unsigned int DisplayEnable                   : 1; // (0 = Off, 1 = On)
                };
            };
            #pragma pack(1)
            union
            {
                uint8_t GPUStatus;           // 0xFF41

                #pragma pack(1)
                struct
                {
                    unsigned int GPUMode              : 2;
                    unsigned int Coincidence          : 1;
                    unsigned int HBlankInterrupt      : 1;
                    unsigned int VBlankInterrupt      : 1;
                    unsigned int OAMInterrupt         : 1;
                    unsigned int CoincidenceInterrupt : 1;
                    unsigned int                      : 1;
                };
            };
            uint8_t ScrollY;                 // 0xFF42
            uint8_t ScrollX;                 // 0xFF43
            uint8_t Scanline;                // 0xFF44
            uint8_t ScanlineCompare;         // 0xFF45
            uint8_t OAMTransferStartAddress; // 0xFF46
            #pragma pack(1)
            union
            {
                uint8_t BackgroundPalette;   // 0xFF47                                           - Non GBC mode only

                #pragma pack(1)
                struct
                {
                    unsigned int BackgroundPaletteColor0 : 2;
                    unsigned int BackgroundPaletteColor1 : 2;
                    unsigned int BackgroundPaletteColor2 : 2;
                    unsigned int BackgroundPaletteColor3 : 2;
                };
            };
            #pragma pack(1)
            union
            {
                uint8_t ObjectPalette0;      // 0xFF48                                           - Non GBC mode only

                #pragma pack(1)
                struct
                {
                    unsigned int ObjectPalette0Color0 : 2;
                    unsigned int ObjectPalette0Color1 : 2;
                    unsigned int ObjectPalette0Color2 : 2;
                    unsigned int ObjectPalette0Color3 : 2;
                };
            };
            #pragma pack(1)
            union
            {
                uint8_t ObjectPalette1;      // 0xFF49                                           - Non GBC mode only

                #pragma pack(1)
                struct
                {
                    unsigned int ObjectPalette1Color0 : 2;
                    unsigned int ObjectPalette1Color1 : 2;
                    unsigned int ObjectPalette1Color2 : 2;
                    unsigned int ObjectPalette1Color3 : 2;
                };
            };
            uint8_t WindowPositionY;         // 0xFF4A
            uint8_t WindowPositionXMinus7;   // 0xFF4B
            uint8_t IO_Unk5;
            #pragma pack(1)
            union
            {
                uint8_t SpeedSwitch;         // 0xFF4D                                           - Only in GBC mode

                #pragma pack(1)
                struct
                {
                    unsigned int PrepareSpeedSwitch : 1; // (0 = No, 1 = Prepare)
                    unsigned int                    : 6;
                    unsigned int CurrentSpeed       : 1; // (0 = Normal, 1 = Double)
                };
            };
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
    uint8_t InterruptEnable;              // FFFF:        1B Interrupt enable register
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

typedef void (*GBC_MMU_MBC)(uint16_t, uint8_t);

extern GBC_MMU_Memory_t GBC_MMU_Memory;                             // External GBC Memory definition for direct CPU access

extern GBC_MMU_MemoryBankController_t GBC_MMU_MemoryBankController; // Current Memory Bank Controller
extern GBC_MMU_MBC1Mode_t GBC_MMU_MBC1Mode;                         // ROM/RAM Mode Select
extern uint16_t GBC_MMU_CurrentROMBankID;                           // Current ROM Bank ID
extern uint32_t GBC_MMU_CurrentROMBankAddress;                      // Current ROM Bank Start Address

extern bool GBC_MMU_ERAMEnabled;                                    // External RAM Enabled State
extern uint8_t GBC_MMU_CurrentERAMBankID;                           // Current ERAM Bank ID

extern bool GBC_MMU_RTC_Selected;                                   // External RTC Selected State
extern GBC_MMU_RTC_Register_t GBC_MMU_RTC_Register;                 // External RTC Register
extern uint8_t GBC_MMU_RTC_RegisterID;                              // External RTC Register ID

extern GBC_MMU_MBC GBC_MMU_MBC_Table[6];

bool GBC_MMU_LoadFromCartridge(void);
bool GBC_MMU_LoadFromSDC(char* fileName);
void GBC_MMU_Unload(void);

uint8_t GBC_MMU_ReadByte(uint16_t address);
uint16_t GBC_MMU_ReadShort(uint16_t address);

void GBC_MMU_WriteByte(uint16_t address, uint8_t value);
void GBC_MMU_WriteShort(uint16_t address, uint16_t value);

GBC_MMU_MemoryBankController_t GBC_MMU_GetMemoryBankController(void);

static void GBC_MMU_Debug()
{
    NOP;
}

#define GBC_MMU_WRITE_BYTE(ADDRESS, VALUE)                                                                                            \
{                                                                                                                                     \
    switch ((ADDRESS) & 0xF000)                                                                                                       \
    {                                                                                                                                 \
        case 0x0000:                                                                                                                  \
        case 0x1000:                                                                                                                  \
        case 0x2000:                                                                                                                  \
        case 0x3000:                                                                                                                  \
        case 0x4000:                                                                                                                  \
        case 0x5000:                                                                                                                  \
        case 0x6000:                                                                                                                  \
        case 0x7000:                                                                                                                  \
        {                                                                                                                             \
            /* Memory Bank Switch */                                                                                                  \
            if (GBC_LoadState == GBC_LOAD_STATE_CARTRIDGE)                                                                            \
            {                                                                                                                         \
                CMOD_WriteByte((ADDRESS), &(VALUE));                                                                                  \
                while (CMOD_GetStatus() == CMOD_PROCESSING);                                                                          \
            }                                                                                                                         \
            else /* Loaded from SDC */                                                                                                \
            {                                                                                                                         \
                GBC_MMU_MBC_Table[GBC_MMU_MemoryBankController]((ADDRESS), (VALUE));                                                  \
            }                                                                                                                         \
            break;                                                                                                                    \
        }                                                                                                                             \
        case 0x8000:                                                                                                                  \
        case 0x9000:                                                                                                                  \
        {                                                                                                                             \
            /* Video RAM bank X */                                                                                                    \
            if (GBC_MMU_Memory.VRAMBankID == 1)                                                                                       \
            {                                                                                                                         \
                GBC_MMU_Memory.VRAMBank1[(ADDRESS) - 0x8000] = (VALUE);                                                               \
            }                                                                                                                         \
            else                                                                                                                      \
            {                                                                                                                         \
                GBC_MMU_Memory.VRAMBank0[(ADDRESS) - 0x8000] = (VALUE);                                                               \
            }                                                                                                                         \
            break;                                                                                                                    \
        }                                                                                                                             \
        case 0xA000:                                                                                                                  \
        case 0xB000:                                                                                                                  \
        {                                                                                                                             \
            /* External RAM bank X */                                                                                                 \
            if (GBC_LoadState == GBC_LOAD_STATE_CARTRIDGE)                                                                            \
            {                                                                                                                         \
                CMOD_WriteByte((ADDRESS), &(VALUE));                                                                                  \
                while (CMOD_GetStatus() == CMOD_PROCESSING);                                                                          \
            }                                                                                                                         \
            else if (GBC_MMU_ERAMEnabled)                                                                                             \
            {                                                                                                                         \
                if (GBC_MMU_RTC_Selected)                                                                                             \
                {                                                                                                                     \
                    GBC_MMU_RTC_Register.Data[GBC_MMU_RTC_RegisterID] = (VALUE);                                                      \
                }                                                                                                                     \
                else switch (GBC_MMU_CurrentERAMBankID)                                                                               \
                {                                                                                                                     \
                    case 0:                                                                                                           \
                        GBC_MMU_Memory.ERAMBank0[(ADDRESS) - 0xA000] = (VALUE);                                                       \
                        break;                                                                                                        \
                    case 1:                                                                                                           \
                        GBC_MMU_Memory.ERAMBank1[(ADDRESS) - 0xA000] = (VALUE);                                                       \
                        break;                                                                                                        \
                    case 2:                                                                                                           \
                        GBC_MMU_Memory.ERAMBank2[(ADDRESS) - 0xA000] = (VALUE);                                                       \
                        break;                                                                                                        \
                    case 3:                                                                                                           \
                        GBC_MMU_Memory.ERAMBank3[(ADDRESS) - 0xA000] = (VALUE);                                                       \
                        break;                                                                                                        \
                }                                                                                                                     \
            }                                                                                                                         \
            break;                                                                                                                    \
        }                                                                                                                             \
        case 0xC000:                                                                                                                  \
        {                                                                                                                             \
            /* Work RAM Bank 0 */                                                                                                     \
            GBC_MMU_Memory.WRAMBank0[(ADDRESS) - 0xC000] = (VALUE);                                                                   \
            break;                                                                                                                    \
        }                                                                                                                             \
        case 0xD000:                                                                                                                  \
        {                                                                                                                             \
            /* Work RAM Bank X */                                                                                                     \
            switch (GBC_MMU_Memory.WRAMBankID)                                                                                        \
            {                                                                                                                         \
                case 0:                                                                                                               \
                case 1:                                                                                                               \
                    GBC_MMU_Memory.WRAMBank1[(ADDRESS) - 0xD000] = (VALUE);                                                           \
                    break;                                                                                                            \
                case 2:                                                                                                               \
                    GBC_MMU_Memory.WRAMBank2[(ADDRESS) - 0xD000] = (VALUE);                                                           \
                    break;                                                                                                            \
                case 3:                                                                                                               \
                    GBC_MMU_Memory.WRAMBank3[(ADDRESS) - 0xD000] = (VALUE);                                                           \
                    break;                                                                                                            \
                case 4:                                                                                                               \
                    GBC_MMU_Memory.WRAMBank4[(ADDRESS) - 0xD000] = (VALUE);                                                           \
                    break;                                                                                                            \
                case 5:                                                                                                               \
                    GBC_MMU_Memory.WRAMBank5[(ADDRESS) - 0xD000] = (VALUE);                                                           \
                    break;                                                                                                            \
                case 6:                                                                                                               \
                    GBC_MMU_Memory.WRAMBank6[(ADDRESS) - 0xD000] = (VALUE);                                                           \
                    break;                                                                                                            \
                case 7:                                                                                                               \
                    GBC_MMU_Memory.WRAMBank7[(ADDRESS) - 0xD000] = (VALUE);                                                           \
                    break;                                                                                                            \
                default:                                                                                                              \
                    GBC_MMU_Memory.WRAMBank1[(ADDRESS) - 0xD000] = (VALUE);                                                           \
                    break;                                                                                                            \
            }                                                                                                                         \
            break;                                                                                                                    \
        }                                                                                                                             \
        case 0xE000:                                                                                                                  \
        {                                                                                                                             \
            /* Shadow RAM redirection to WRAM */                                                                                      \
            GBC_MMU_Memory.WRAMBank0[(ADDRESS) - 0xE000] = (VALUE);                                                                   \
            break;                                                                                                                    \
        }                                                                                                                             \
        case 0xF000:                                                                                                                  \
        {                                                                                                                             \
            switch ((ADDRESS))                                                                                                        \
            {                                                                                                                         \
                /* Object Attribute Memory */                                                                                         \
                case 0xFE00: case 0xFE01: case 0xFE02: case 0xFE03: case 0xFE04: case 0xFE05: case 0xFE06: case 0xFE07:               \
                case 0xFE08: case 0xFE09: case 0xFE0A: case 0xFE0B: case 0xFE0C: case 0xFE0D: case 0xFE0E: case 0xFE0F:               \
                case 0xFE10: case 0xFE11: case 0xFE12: case 0xFE13: case 0xFE14: case 0xFE15: case 0xFE16: case 0xFE17:               \
                case 0xFE18: case 0xFE19: case 0xFE1A: case 0xFE1B: case 0xFE1C: case 0xFE1D: case 0xFE1E: case 0xFE1F:               \
                case 0xFE20: case 0xFE21: case 0xFE22: case 0xFE23: case 0xFE24: case 0xFE25: case 0xFE26: case 0xFE27:               \
                case 0xFE28: case 0xFE29: case 0xFE2A: case 0xFE2B: case 0xFE2C: case 0xFE2D: case 0xFE2E: case 0xFE2F:               \
                case 0xFE30: case 0xFE31: case 0xFE32: case 0xFE33: case 0xFE34: case 0xFE35: case 0xFE36: case 0xFE37:               \
                case 0xFE38: case 0xFE39: case 0xFE3A: case 0xFE3B: case 0xFE3C: case 0xFE3D: case 0xFE3E: case 0xFE3F:               \
                case 0xFE40: case 0xFE41: case 0xFE42: case 0xFE43: case 0xFE44: case 0xFE45: case 0xFE46: case 0xFE47:               \
                case 0xFE48: case 0xFE49: case 0xFE4A: case 0xFE4B: case 0xFE4C: case 0xFE4D: case 0xFE4E: case 0xFE4F:               \
                case 0xFE50: case 0xFE51: case 0xFE52: case 0xFE53: case 0xFE54: case 0xFE55: case 0xFE56: case 0xFE57:               \
                case 0xFE58: case 0xFE59: case 0xFE5A: case 0xFE5B: case 0xFE5C: case 0xFE5D: case 0xFE5E: case 0xFE5F:               \
                case 0xFE60: case 0xFE61: case 0xFE62: case 0xFE63: case 0xFE64: case 0xFE65: case 0xFE66: case 0xFE67:               \
                case 0xFE68: case 0xFE69: case 0xFE6A: case 0xFE6B: case 0xFE6C: case 0xFE6D: case 0xFE6E: case 0xFE6F:               \
                case 0xFE70: case 0xFE71: case 0xFE72: case 0xFE73: case 0xFE74: case 0xFE75: case 0xFE76: case 0xFE77:               \
                case 0xFE78: case 0xFE79: case 0xFE7A: case 0xFE7B: case 0xFE7C: case 0xFE7D: case 0xFE7E: case 0xFE7F:               \
                case 0xFE80: case 0xFE81: case 0xFE82: case 0xFE83: case 0xFE84: case 0xFE85: case 0xFE86: case 0xFE87:               \
                case 0xFE88: case 0xFE89: case 0xFE8A: case 0xFE8B: case 0xFE8C: case 0xFE8D: case 0xFE8E: case 0xFE8F:               \
                case 0xFE90: case 0xFE91: case 0xFE92: case 0xFE93: case 0xFE94: case 0xFE95: case 0xFE96: case 0xFE97:               \
                case 0xFE98: case 0xFE99: case 0xFE9A: case 0xFE9B: case 0xFE9C: case 0xFE9D: case 0xFE9E: case 0xFE9F:               \
                    GBC_MMU_Memory.OAM[(ADDRESS) - 0xFE00] = (VALUE);                                                                 \
                    break;                                                                                                            \
                /* Unused Memory */                                                                                                   \
                case 0xFEA0: case 0xFEA1: case 0xFEA2: case 0xFEA3: case 0xFEA4: case 0xFEA5: case 0xFEA6: case 0xFEA7:               \
                case 0xFEA8: case 0xFEA9: case 0xFEAA: case 0xFEAB: case 0xFEAC: case 0xFEAD: case 0xFEAE: case 0xFEAF:               \
                case 0xFEB0: case 0xFEB1: case 0xFEB2: case 0xFEB3: case 0xFEB4: case 0xFEB5: case 0xFEB6: case 0xFEB7:               \
                case 0xFEB8: case 0xFEB9: case 0xFEBA: case 0xFEBB: case 0xFEBC: case 0xFEBD: case 0xFEBE: case 0xFEBF:               \
                case 0xFEC0: case 0xFEC1: case 0xFEC2: case 0xFEC3: case 0xFEC4: case 0xFEC5: case 0xFEC6: case 0xFEC7:               \
                case 0xFEC8: case 0xFEC9: case 0xFECA: case 0xFECB: case 0xFECC: case 0xFECD: case 0xFECE: case 0xFECF:               \
                case 0xFED0: case 0xFED1: case 0xFED2: case 0xFED3: case 0xFED4: case 0xFED5: case 0xFED6: case 0xFED7:               \
                case 0xFED8: case 0xFED9: case 0xFEDA: case 0xFEDB: case 0xFEDC: case 0xFEDD: case 0xFEDE: case 0xFEDF:               \
                case 0xFEE0: case 0xFEE1: case 0xFEE2: case 0xFEE3: case 0xFEE4: case 0xFEE5: case 0xFEE6: case 0xFEE7:               \
                case 0xFEE8: case 0xFEE9: case 0xFEEA: case 0xFEEB: case 0xFEEC: case 0xFEED: case 0xFEEE: case 0xFEEF:               \
                case 0xFEF0: case 0xFEF1: case 0xFEF2: case 0xFEF3: case 0xFEF4: case 0xFEF5: case 0xFEF6: case 0xFEF7:               \
                case 0xFEF8: case 0xFEF9: case 0xFEFA: case 0xFEFB: case 0xFEFC: case 0xFEFD: case 0xFEFE: case 0xFEFF:               \
                    /* Do nothing */                                                                                                  \
                    break;                                                                                                            \
                /* Memory-mapped I/O */                                                                                               \
                case 0xFF00: /* Joypad */                                                                                             \
                    GBC_MMU_Memory.Joypad = (VALUE);                                                                                  \
                    Input_UpdateJoypadState();                                                                                        \
                    break;                                                                                                            \
                case 0xFF01:                                                                                                          \
                case 0xFF02:                                                                                                          \
                case 0xFF03:                                                                                                          \
                    GBC_MMU_Memory.IO[(ADDRESS) - 0xFF00] = (VALUE);                                                                  \
                    break;                                                                                                            \
                case 0xFF04: /* Timer Divider: Writing any (VALUE) to this register resets it to 0 */                                 \
                    GBC_TIM_ResetDivider();                                                                                           \
                    break;                                                                                                            \
                case 0xFF05:                                                                                                          \
                case 0xFF06:                                                                                                          \
                    GBC_MMU_Memory.IO[(ADDRESS) - 0xFF00] = (VALUE);                                                                  \
                    break;                                                                                                            \
                case 0xFF07: /* Timer Control */                                                                                      \
                    if (GBC_MMU_Memory.TimerRunning != ((VALUE) & 0x03))                                                              \
                    {                                                                                                                 \
                        GBC_TIM_ResetCounter();                                                                                       \
                    }                                                                                                                 \
                                                                                                                                      \
                    GBC_MMU_Memory.TimerControl = (VALUE) & 0x07;                                                                     \
                    break;                                                                                                            \
                case 0xFF08:                                                                                                          \
                case 0xFF09:                                                                                                          \
                case 0xFF0A:                                                                                                          \
                case 0xFF0B:                                                                                                          \
                case 0xFF0C:                                                                                                          \
                case 0xFF0D:                                                                                                          \
                case 0xFF0E:                                                                                                          \
                case 0xFF0F:                                                                                                          \
                case 0xFF10:                                                                                                          \
                case 0xFF11:                                                                                                          \
                case 0xFF12:                                                                                                          \
                case 0xFF13:                                                                                                          \
                case 0xFF14:                                                                                                          \
                case 0xFF15:                                                                                                          \
                case 0xFF16:                                                                                                          \
                case 0xFF17:                                                                                                          \
                case 0xFF18:                                                                                                          \
                case 0xFF19:                                                                                                          \
                case 0xFF1A:                                                                                                          \
                case 0xFF1B:                                                                                                          \
                case 0xFF1C:                                                                                                          \
                case 0xFF1D:                                                                                                          \
                case 0xFF1E:                                                                                                          \
                case 0xFF1F:                                                                                                          \
                case 0xFF20:                                                                                                          \
                case 0xFF21:                                                                                                          \
                case 0xFF22:                                                                                                          \
                case 0xFF23:                                                                                                          \
                case 0xFF24:                                                                                                          \
                case 0xFF25:                                                                                                          \
                case 0xFF26:                                                                                                          \
                case 0xFF27:                                                                                                          \
                case 0xFF28:                                                                                                          \
                case 0xFF29:                                                                                                          \
                case 0xFF2A:                                                                                                          \
                case 0xFF2B:                                                                                                          \
                case 0xFF2C:                                                                                                          \
                case 0xFF2D:                                                                                                          \
                case 0xFF2E:                                                                                                          \
                case 0xFF2F:                                                                                                          \
                case 0xFF30:                                                                                                          \
                case 0xFF31:                                                                                                          \
                case 0xFF32:                                                                                                          \
                case 0xFF33:                                                                                                          \
                case 0xFF34:                                                                                                          \
                case 0xFF35:                                                                                                          \
                case 0xFF36:                                                                                                          \
                case 0xFF37:                                                                                                          \
                case 0xFF38:                                                                                                          \
                case 0xFF39:                                                                                                          \
                case 0xFF3A:                                                                                                          \
                case 0xFF3B:                                                                                                          \
                case 0xFF3C:                                                                                                          \
                case 0xFF3D:                                                                                                          \
                case 0xFF3E:                                                                                                          \
                case 0xFF3F:                                                                                                          \
                case 0xFF40:                                                                                                          \
                case 0xFF41:                                                                                                          \
                case 0xFF42:                                                                                                          \
                case 0xFF43:                                                                                                          \
                case 0xFF44:                                                                                                          \
                case 0xFF45:                                                                                                          \
                    GBC_MMU_Memory.IO[(ADDRESS) - 0xFF00] = (VALUE);                                                                  \
                    break;                                                                                                            \
                case 0xFF46: /* DMA Transfer and Start Address */                                                                     \
                    GBC_MMU_Memory.OAMTransferStartAddress = (VALUE);                                                                 \
                                                                                                                                      \
                    if (GBC_MMU_Memory.CGBFlag & (GBC_MMU_CGB_FLAG_SUPPORTED | GBC_MMU_CGB_FLAG_ONLY))                                \
                    {                                                                                                                 \
                        /* ToDo */                                                                                                    \
                    }                                                                                                                 \
                    else                                                                                                              \
                    {                                                                                                                 \
                        /* The written value specifies the transfer source address divided by 100h */                                 \
                        uint16_t address = (VALUE) << 8;                                                                              \
                                                                                                                                      \
                        if (address >= 0x8000 && address < 0xE000)                                                                    \
                        {                                                                                                             \
                            for (uint32_t i = 0; i < 160; i++, address++)                                                             \
                            {                                                                                                         \
                                GBC_MMU_Memory.OAM[i] = GBC_MMU_ReadByte(address);                                                    \
                            }                                                                                                         \
                        }                                                                                                             \
                    }                                                                                                                 \
                    break;                                                                                                            \
                case 0xFF47:                                                                                                          \
                case 0xFF48:                                                                                                          \
                case 0xFF49:                                                                                                          \
                case 0xFF4A:                                                                                                          \
                case 0xFF4B:                                                                                                          \
                case 0xFF4C:                                                                                                          \
                case 0xFF4D:                                                                                                          \
                case 0xFF4E:                                                                                                          \
                case 0xFF4F:                                                                                                          \
                case 0xFF50:                                                                                                          \
                case 0xFF51:                                                                                                          \
                case 0xFF52:                                                                                                          \
                case 0xFF53:                                                                                                          \
                case 0xFF54:                                                                                                          \
                case 0xFF55:                                                                                                          \
                case 0xFF56:                                                                                                          \
                case 0xFF57:                                                                                                          \
                case 0xFF58:                                                                                                          \
                case 0xFF59:                                                                                                          \
                case 0xFF5A:                                                                                                          \
                case 0xFF5B:                                                                                                          \
                case 0xFF5C:                                                                                                          \
                case 0xFF5D:                                                                                                          \
                case 0xFF5E:                                                                                                          \
                case 0xFF5F:                                                                                                          \
                case 0xFF60:                                                                                                          \
                case 0xFF61:                                                                                                          \
                case 0xFF62:                                                                                                          \
                case 0xFF63:                                                                                                          \
                case 0xFF64:                                                                                                          \
                case 0xFF65:                                                                                                          \
                case 0xFF66:                                                                                                          \
                case 0xFF67:                                                                                                          \
                case 0xFF68:                                                                                                          \
                case 0xFF69:                                                                                                          \
                case 0xFF6A:                                                                                                          \
                case 0xFF6B:                                                                                                          \
                case 0xFF6C:                                                                                                          \
                case 0xFF6D:                                                                                                          \
                case 0xFF6E:                                                                                                          \
                case 0xFF6F:                                                                                                          \
                case 0xFF70:                                                                                                          \
                case 0xFF71:                                                                                                          \
                case 0xFF72:                                                                                                          \
                case 0xFF73:                                                                                                          \
                case 0xFF74:                                                                                                          \
                case 0xFF75:                                                                                                          \
                case 0xFF76:                                                                                                          \
                case 0xFF77:                                                                                                          \
                case 0xFF78:                                                                                                          \
                case 0xFF79:                                                                                                          \
                case 0xFF7A:                                                                                                          \
                case 0xFF7B:                                                                                                          \
                case 0xFF7C:                                                                                                          \
                case 0xFF7D:                                                                                                          \
                case 0xFF7E:                                                                                                          \
                case 0xFF7F:                                                                                                          \
                    GBC_MMU_Memory.IO[(ADDRESS) - 0xFF00] = (VALUE);                                                                  \
                    break;                                                                                                            \
                /* High RAM */                                                                                                        \
                case 0xFF80: case 0xFF81: case 0xFF82: case 0xFF83: case 0xFF84: case 0xFF85: case 0xFF86: case 0xFF87:               \
                case 0xFF88: case 0xFF89: case 0xFF8A: case 0xFF8B: case 0xFF8C: case 0xFF8D: case 0xFF8E: case 0xFF8F:               \
                case 0xFF90: case 0xFF91: case 0xFF92: case 0xFF93: case 0xFF94: case 0xFF95: case 0xFF96: case 0xFF97:               \
                case 0xFF98: case 0xFF99: case 0xFF9A: case 0xFF9B: case 0xFF9C: case 0xFF9D: case 0xFF9E: case 0xFF9F:               \
                case 0xFFA0: case 0xFFA1: case 0xFFA2: case 0xFFA3: case 0xFFA4: case 0xFFA5: case 0xFFA6: case 0xFFA7:               \
                case 0xFFA8: case 0xFFA9: case 0xFFAA: case 0xFFAB: case 0xFFAC: case 0xFFAD: case 0xFFAE: case 0xFFAF:               \
                case 0xFFB0: case 0xFFB1: case 0xFFB2: case 0xFFB3: case 0xFFB4: case 0xFFB5: case 0xFFB6: case 0xFFB7:               \
                case 0xFFB8: case 0xFFB9: case 0xFFBA: case 0xFFBB: case 0xFFBC: case 0xFFBD: case 0xFFBE: case 0xFFBF:               \
                case 0xFFC0: case 0xFFC1: case 0xFFC2: case 0xFFC3: case 0xFFC4: case 0xFFC5: case 0xFFC6: case 0xFFC7:               \
                case 0xFFC8: case 0xFFC9: case 0xFFCA: case 0xFFCB: case 0xFFCC: case 0xFFCD: case 0xFFCE: case 0xFFCF:               \
                case 0xFFD0: case 0xFFD1: case 0xFFD2: case 0xFFD3: case 0xFFD4: case 0xFFD5: case 0xFFD6: case 0xFFD7:               \
                case 0xFFD8: case 0xFFD9: case 0xFFDA: case 0xFFDB: case 0xFFDC: case 0xFFDD: case 0xFFDE: case 0xFFDF:               \
                case 0xFFE0: case 0xFFE1: case 0xFFE2: case 0xFFE3: case 0xFFE4: case 0xFFE5: case 0xFFE6: case 0xFFE7:               \
                case 0xFFE8: case 0xFFE9: case 0xFFEA: case 0xFFEB: case 0xFFEC: case 0xFFED: case 0xFFEE: case 0xFFEF:               \
                case 0xFFF0: case 0xFFF1: case 0xFFF2: case 0xFFF3: case 0xFFF4: case 0xFFF5: case 0xFFF6: case 0xFFF7:               \
                case 0xFFF8: case 0xFFF9: case 0xFFFA: case 0xFFFB: case 0xFFFC: case 0xFFFD: case 0xFFFE:                            \
                    GBC_MMU_Memory.HRAM[(ADDRESS) - 0xFF80] = (VALUE);                                                                \
                    break;                                                                                                            \
                /* Interrupt Enable Register */                                                                                       \
                case 0xFFFF:                                                                                                          \
                    GBC_MMU_Memory.InterruptEnable = (VALUE);                                                                         \
                    break;                                                                                                            \
                default:                                                                                                              \
                    /* Shadow RAM redirection to WRAM Bank X */                                                                       \
                    switch (GBC_MMU_Memory.WRAMBankID)                                                                                \
                    {                                                                                                                 \
                        case 0:                                                                                                       \
                        case 1:                                                                                                       \
                            GBC_MMU_Memory.WRAMBank1[(ADDRESS) - 0xF000] = (VALUE);                                                   \
                            break;                                                                                                    \
                        case 2:                                                                                                       \
                            GBC_MMU_Memory.WRAMBank2[(ADDRESS) - 0xF000] = (VALUE);                                                   \
                            break;                                                                                                    \
                        case 3:                                                                                                       \
                            GBC_MMU_Memory.WRAMBank3[(ADDRESS) - 0xF000] = (VALUE);                                                   \
                            break;                                                                                                    \
                        case 4:                                                                                                       \
                            GBC_MMU_Memory.WRAMBank4[(ADDRESS) - 0xF000] = (VALUE);                                                   \
                            break;                                                                                                    \
                        case 5:                                                                                                       \
                            GBC_MMU_Memory.WRAMBank5[(ADDRESS) - 0xF000] = (VALUE);                                                   \
                            break;                                                                                                    \
                        case 6:                                                                                                       \
                            GBC_MMU_Memory.WRAMBank6[(ADDRESS) - 0xF000] = (VALUE);                                                   \
                            break;                                                                                                    \
                        case 7:                                                                                                       \
                            GBC_MMU_Memory.WRAMBank7[(ADDRESS) - 0xF000] = (VALUE);                                                   \
                            break;                                                                                                    \
                        default:                                                                                                      \
                            GBC_MMU_Memory.WRAMBank1[(ADDRESS) - 0xF000] = (VALUE);                                                   \
                            break;                                                                                                    \
                    }                                                                                                                 \
                    break;                                                                                                            \
            }                                                                                                                         \
            break;                                                                                                                    \
        }                                                                                                                             \
    }                                                                                                                                 \
}                                                                                                                                     \

#define GBC_MMU_WRITE_SHORT(ADDRESS, VALUE)                                                                                           \
{                                                                                                                                     \
    uint16_t GBC_MMU_ADDRESS = (ADDRESS);                                                                                               \
    uint8_t  GBC_MMU_VALUE   = (VALUE) & 0xFF;                                                                                          \
                                                                                                                                      \
    GBC_MMU_WRITE_BYTE(GBC_MMU_ADDRESS, GBC_MMU_VALUE);                                                                               \
                                                                                                                                      \
    GBC_MMU_ADDRESS++;                                                                                                                \
    GBC_MMU_VALUE = ((VALUE) & 0xFF00) >> 8;                                                                                            \
                                                                                                                                      \
    GBC_MMU_WRITE_BYTE(GBC_MMU_ADDRESS, GBC_MMU_VALUE);                                                                               \
}                                                                                                                                     \

#endif
