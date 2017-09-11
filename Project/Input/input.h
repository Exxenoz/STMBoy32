#ifndef INPUT_H
#define INPUT_H

#include "common.h"
#include "input_config.h"

void Input_Initialize(void);

void EXTI0_IRQHandler(void);
void EXTI1_IRQHandler(void);
void EXTI2_IRQHandler(void);
void EXTI3_IRQHandler(void);
void EXTI4_IRQHandler(void);
void EXTI9_5_IRQHandler(void);

#endif
