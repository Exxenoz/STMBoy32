#ifndef CMOD_H
#define CMOD_H

#include "common.h"
#include "led.h"
#include "cmod_config.h"
#include "gbc_mmu.h"

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

CMOD_STATUS CMOD_GetStatus(void); 
bool        CMOD_Detect(void);           

void CMOD_ReadByte(uint16_t address, uint8_t *data);                       
void CMOD_ReadBytes(uint16_t startingAddress, int bytes, uint8_t *data);   
void CMOD_WriteByte(uint16_t address, uint8_t *data);                      
void CMOD_WriteBytes(uint16_t startingAddress, int bytes, uint8_t *data);  

void CMOD_SaveCartridge(void);        // Needed arguments currently unknown
void CMOD_SwitchMB(uint8_t bank);     // Switch active Memory Bank of the cartridge

void CMOD_Initialize(void); 

void TIM4_IRQHandler(void);
void EXTI15_10_IRQHandler(void); 

#endif
