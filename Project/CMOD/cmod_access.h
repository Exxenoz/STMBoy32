#ifndef CMOD_ACCESS_H
#define CMOD_ACCESS_H

#include "common.h"


typedef enum 
{
    CMOD_OK,
    CMOD_TIMEOUT,
    CMOD_TURNEDOFF,
    CMOD_NOCARTRIDGE,
} CMOD_Result_t;

typedef enum 
{
    CMOD_WAITING,
    CMOD_PROCESSING,
    CMOD_DATA_READY,
    CMOD_WRITE_COMPLETE,
    CMOD_TURNED_OFF
} CMOD_Status_t;

typedef enum
{
    CMOD_READ,
    CMOD_WRITE,
    CMOD_NOACTION
} CMOD_Action_t;



CMOD_Status_t CMOD_GetStatus(void);

void CMOD_TurnON(void);
void CMOD_TurnOFF(void);
void CMOD_ResetCartridge(void);

CMOD_Result_t CMOD_ReadByte(uint16_t address, uint8_t *data);                       
CMOD_Result_t CMOD_ReadBytes(uint16_t startingAddress, int bytes, uint8_t *data);   
CMOD_Result_t CMOD_WriteByte(uint16_t address, uint8_t *data);                      
CMOD_Result_t CMOD_WriteBytes(uint16_t startingAddress, int bytes, uint8_t *data);  

void TIM4_IRQHandler(void);

#endif //CMOD_ACCESS_H
