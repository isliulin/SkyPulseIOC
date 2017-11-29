#include "gpio.h"
#include "isr.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
/**
******************************************************************************
* @file
* @author  Fotona d.d.
* @version
* @date
* @brief	 
*
*/
/** @addtogroup
* @{
*/
/*******************************************************************************
* Function Name	: GPIO() 
* Description		: Footswitch port constructor
* Output				:
* Return				: None
*******************************************************************************/	
_GPIO::_GPIO() {
			GPIO_InitTypeDef	GPIO_InitStructure;

			GPIO_StructInit(&GPIO_InitStructure);
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
			GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;


#if defined (_12Voff_PIN)
			GPIO_InitStructure.GPIO_Pin = _12Voff_PIN;
			GPIO_Init(_12Voff_PORT, &GPIO_InitStructure);
			GPIO_ResetBits(_12Voff_PORT,_12Voff_PIN);
#endif
#if defined (_SYS_SHG_PIN)
			GPIO_InitStructure.GPIO_Pin = _SYS_SHG_PIN;
			GPIO_Init(_SYS_SHG_PORT, &GPIO_InitStructure);
			GPIO_ResetBits(_SYS_SHG_PORT,_SYS_SHG_PIN);
#endif
#if defined (_PILOT_PIN)
			GPIO_InitStructure.GPIO_Pin = _PILOT_PIN;
			GPIO_Init(_PILOT_PORT, &GPIO_InitStructure);
			GPIO_SetBits(_PILOT_PORT,_PILOT_PIN);
#endif
#if defined (_SYS_SHG_sense_PIN)
			GPIO_StructInit(&GPIO_InitStructure);
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
			GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
			GPIO_InitStructure.GPIO_Pin = _SYS_SHG_sense_PIN;
			GPIO_Init(_SYS_SHG_sense_PORT, &GPIO_InitStructure);
#endif	
#if defined (_FOOT_MASK)
			GPIO_StructInit(&GPIO_InitStructure);
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
			GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
			GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
			GPIO_InitStructure.GPIO_Pin = _FOOT_MASK;
			GPIO_Init(_FOOT_PORT, &GPIO_InitStructure);
			GPIO_SetBits(_FOOT_PORT, _FOOT_MASK);	
#endif
			timeout=0;
			key = temp = GPIO_ReadInputData(GPIOC) & _FOOT_MASK;
}
/*******************************************************************************
* Function Name	: Poll()
* Description		:	footswitch port polling
* Output				: int
* Return				: footswitch code, 20ms filter, on valid change
*******************************************************************************/
int   _GPIO::Poll(void) {
			if(temp != (GPIO_ReadInputData(GPIOC) & _FOOT_MASK)) {
				temp = GPIO_ReadInputData(GPIOC) & _FOOT_MASK;
				timeout = __time__ + 5;
			} else 
					if(timeout && __time__ > timeout) {
						timeout=0;
						if(temp != key) {
							key=temp;
							return key;
						}
			}
			return EOF;
}
/**
* @}
*/
