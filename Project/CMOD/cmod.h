#ifndef CMOD_H
#define CMOD_H

#include "common.h"
#include "led.h"
#include "cmod_config.h"

#define CMOD_SET_RESET   CMOD_RESET_PORT->BSRRL |= CMOD_RESET_PIN
#define CMOD_RST_RESET   CMOD_RESET_PORT->BSRRH |= CMOD_RESET_PIN

#define CMOD_SET_CS      CMOD_CS_PORT->BSRRL |= CMOD_CS_PIN
#define CMOD_RST_CS      CMOD_CS_PORT->BSRRH |= CMOD_CS_PIN

#define CMOD_SET_RD      CMOD_RD_PORT->BSRRL |= CMOD_RD_PIN
#define CMOD_RST_RD      CMOD_RD_PORT->BSRRH |= CMOD_RD_PIN

#define CMOD_SET_WR      CMOD_WR_PORT->BSRRL |= CMOD_WR_PIN
#define CMOD_RST_WR      CMOD_WR_PORT->BSRRH |= CMOD_WR_PIN

#define CMOD_SET_ADDR(ADDR) CMOD_ADDR_PORT->ODR = ADDR
#define CMOD_SET_DATA(DATA) CMOD_DATA_PORT->ODR = DATA
#define CMOD_GET_DATA()     CMOD_DATA_PORT->IDR & 0xFF

void CMOD_Initialize(void);
void CMOD_Initialize_Timer(void);
void CMOD_Initialize_PWM(void);
void CMOD_Initialize_CLK_Interrupt(void);
void CMOD_Initialize_Insertion_Interrupt(void);
void EXTI15_10_IRQHandler(void); 

#endif
