#ifndef CMOD_ACCESS_H
#define CMOD_ACCESS_H

#include "cmod.h"



CMOD_Status_t CMOD_GetStatus(void);

void CMOD_ResetCartridge(void);
void CMOD_ReadByte(uint16_t address, uint8_t *data);                       
void CMOD_ReadBytes(uint16_t startingAddress, int bytes, uint8_t *data);   
void CMOD_WriteByte(uint16_t address, uint8_t *data);                      
void CMOD_WriteBytes(uint16_t startingAddress, int bytes, uint8_t *data);  

void TIM5_IRQHandler(void);

#endif //CMOD_ACCESS_H
