#ifndef CMOD_H
#define CMOD_H

#include "common.h"
#include "led.h"
#include "cmod_config.h"

#define CMOD_SET_RESET       CMOD_RESET_PORT->BSRRL |= CMOD_RESET_PIN
#define CMOD_RST_RESET       CMOD_RESET_PORT->BSRRH |= CMOD_RESET_PIN
                             
#define CMOD_SET_CS          CMOD_CS_PORT->BSRRL |= CMOD_CS_PIN
#define CMOD_RST_CS          CMOD_CS_PORT->BSRRH |= CMOD_CS_PIN
                             
#define CMOD_SET_RD          CMOD_RD_PORT->BSRRL |= CMOD_RD_PIN
#define CMOD_RST_RD          CMOD_RD_PORT->BSRRH |= CMOD_RD_PIN
                             
#define CMOD_SET_WR          CMOD_WR_PORT->BSRRL |= CMOD_WR_PIN
#define CMOD_RST_WR          CMOD_WR_PORT->BSRRH |= CMOD_WR_PIN
                             
#define CMOD_SET_ADDR(ADDR)  CMOD_ADDR_PORT->ODR = ADDR
#define CMOD_SET_DATA(DATA)  CMOD_DATA_PORT->ODR = DATA
#define CMOD_GET_DATA()      CMOD_DATA_PORT->IDR & 0xFF
                             
#define CMOD_DATA_MODE_IN    GPIOG->MODER = 0x00000000
#define CMOD_DATA_MODE_OUT   GPIOG->MODER = 0x00005555

typedef enum 
{
    CMOD_WAITING,
    CMOD_PROCESSING,
    CMOD_DATA_READY,
    CMOD_WRITE_COMPLETE,
    CMOD_NOCARD
} CMOD_STATUS;

typedef enum
{
    CMOD_READ,
    CMOD_WRITE,
    CMOD_NOACTION
} CMOD_ACTION;

typedef enum
{
    C_MBC_UNKNOWN,             // The cardridge's MBC is unknown
    C_NO_MBC,                  // The cardridge only has ROM (Tetris for example)
    C_MBC1,                    // The cardridge has MC1   build in
    C_MBC2,                    // The cardridge has MC2   build in
    C_MBC3,                    // The cardridge has MC3   build in
    C_MBC5,                    // The cardridge has MC5   build in
    C_MBC6,                    // The cardridge has MC6   build in
    C_MBC7,                    // The cardridge has MC7   build in
    C_HuC1,                    // The cardridge has HuC1  build in (?)
    C_HuC3,                    // The cardridge has HuC3  build in (?)
    C_MMM01,                   // The cardridge has MMM01 build in (?)
    C_BANDAI                   // Bandai Tama5, apparently only used in Tamagochi 
} CARTRIDGE_MBC;

typedef struct
{
    uint8_t       C_Title[11];       // Title of the Game
    CARTRIDGE_MBC C_Mbc;             // Which Memory Block Controller is used by the cartridge
    bool          C_PocketCamera;    // The GameBoy Pocket Camera is currently inserted
    bool          C_GBCGame;         // False if the Game doesn't support GBC functions
    bool          C_Battery;         // Is there a battery build into the cartridge
    bool          C_Timer;           // Is there a battery build into the cartridge
    bool          C_Rumble;          // Is there rumble build into the cartridge (?)
    bool          C_Sensor;          // Is a sensor build into the crtridge (only MC7) (?)
    int           C_KByteROM;        // How much KByte ROM does the Cartridge have
    int           C_ROMBanks;        // How many ROM Banks does the cartridge have
    int           C_KByteRAM;        // How much KByte RAM does the Cartridge have
    int           C_RAMBanks;        // How many RAM Banks does the cartridge have (0 for MBC2, has 512x4bit tho)
} CARTRIDGE_SPECS;

CMOD_STATUS CMOD_GetStatus(void); 
bool        CMOD_Detect(void);           

void CMOD_ReadByte(uint16_t address, uint8_t *data);                       
void CMOD_ReadBytes(uint16_t startingAddress, int bytes, uint8_t *data);   
void CMOD_WriteByte(uint16_t address, uint8_t *data);                      
void CMOD_WriteBytes(uint16_t startingAddress, int bytes, uint8_t *data);  

void CMOD_Initialize(void);
void CMOD_EnableInterrupt(void);  
void CMOD_DisableInterrupt(void); 

void CMOD_GetCSpecs(CARTRIDGE_SPECS *cSpecs);
void C_Initialize(CARTRIDGE_SPECS *cSpecs);
void C_GetType(uint8_t type, CARTRIDGE_SPECS *cSpecs);
void C_GetROM(uint8_t rom, CARTRIDGE_SPECS *cSpecs);
void C_GetRAM(uint8_t ram, CARTRIDGE_SPECS *cSpecs);
    
void TIM4_IRQHandler(void);
void EXTI15_10_IRQHandler(void); 

#endif
