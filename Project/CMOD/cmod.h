#ifndef CMOD_H
#define CMOD_H

#include "common.h"
#include "cmod_config.h"


#define CMOD_SET_RESET            CMOD_RESET_PORT->BSRRL |= CMOD_RESET_PIN
#define CMOD_RST_RESET            CMOD_RESET_PORT->BSRRH |= CMOD_RESET_PIN
                             
#define CMOD_SET_CS               CMOD_CS_PORT->BSRRL |= CMOD_CS_PIN
#define CMOD_RST_CS               CMOD_CS_PORT->BSRRH |= CMOD_CS_PIN
                             
#define CMOD_SET_RD               CMOD_RD_PORT->BSRRL |= CMOD_RD_PIN
#define CMOD_RST_RD               CMOD_RD_PORT->BSRRH |= CMOD_RD_PIN
                             
#define CMOD_SET_WR               CMOD_WR_PORT->BSRRL |= CMOD_WR_PIN
#define CMOD_RST_WR               CMOD_WR_PORT->BSRRH |= CMOD_WR_PIN

#define CMOD_SET_ADDR(ADDR)       CMOD_ADDR_PORT->ODR = ADDR
#define CMOD_SET_DATA(DATA)       CMOD_DATA_PORT->ODR = DATA
#define CMOD_GET_DATA()           (CMOD_DATA_PORT->IDR & 0xFF)

#define CMOD_ENABLE_LLC()         CMOD_LLC_PORT->BSRRH |= CMOD_LLC_OE_PIN
#define CMOD_DISABLE_LLC()        CMOD_LLC_PORT->BSRRL |= CMOD_LLC_OE_PIN

#define CMOD_DATA_MODE_IN()       GPIOG->MODER = 0x00000000; CMOD_LLC_PORT->BSRRH |= CMOD_LLC_DD_PIN
#define CMOD_DATA_MODE_OUT()      GPIOG->MODER = 0x00005555; CMOD_LLC_PORT->BSRRL |= CMOD_LLC_DD_PIN

#define CMOD_ENABLE_INTERRUPT()   CMOD_TIM->DIER |= (uint16_t)TIM_IT_UPDATE


typedef enum 
{
    CMOD_WAITING,
    CMOD_PROCESSING,
    CMOD_DATA_READY,
    CMOD_WRITE_COMPLETE
} CMOD_Status_t;

typedef enum
{
    CMOD_READ,
    CMOD_WRITE,
    CMOD_NOACTION
} CMOD_Action_t;

typedef enum
{
    CMOD_SUCCESS,
    CMOD_EXISTS,
    CMOD_NOCARD,
    CMOD_FAILED
} CMOD_SaveResult_t;



bool              CMOD_CheckForCartridge(void);           
CMOD_SaveResult_t CMOD_SaveCartridge(bool overrideExisting);        

#endif
