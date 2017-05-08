#ifndef __SN8200_HAL_H
#define __SN8200_HAL_H
#include <stdbool.h>
#if defined  (STM32F2XX)
#include		"stm32f2xx.h"
#elif defined (STM32F10X_HD)
#include		"stm32f10x.h"
#elif	undefined (STM32F2XX || STM32F10X_HD)
*** undefined target !!!!
#endif

void SN8200_HAL_Init(uint32_t baudrate);
void SN8200_HAL_SendData(unsigned char *buf, int len);
bool SN8200_HAL_RxBufferEmpty(void);
uint8_t SN8200_HAL_ReadByte(void);
#endif
