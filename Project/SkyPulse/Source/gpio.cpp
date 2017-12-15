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

#if defined (_PILOT_PIN)
			GPIO_InitStructure.GPIO_Pin = _PILOT_PIN;
			GPIO_Init(_PILOT_PORT, &GPIO_InitStructure);
			GPIO_SetBits(_PILOT_PORT,_PILOT_PIN);
#endif
	
#if defined (_cwbButton)
			GPIO_StructInit(&GPIO_InitStructure);
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
			GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
			GPIO_InitStructure.GPIO_Pin = _cwbButton;
			GPIO_Init(_cwbPort, &GPIO_InitStructure);
#if defined (_cwbDoor)
			GPIO_InitStructure.GPIO_Pin = _cwbDoor;
			GPIO_Init(_cwbPort, &GPIO_InitStructure);
#endif	
#if defined (_cwbHandpc)
			GPIO_InitStructure.GPIO_Pin = _cwbHandpc;
			GPIO_Init(_cwbPort, &GPIO_InitStructure);
#endif	
#endif	

			GPIO_StructInit(&GPIO_InitStructure);
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
			GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
			GPIO_InitStructure.GPIO_Pin = (_FSW0 | _FSW1 | _FSW2);
			GPIO_Init(_FSW_PORT, &GPIO_InitStructure);

#if defined (_SYS_SHG_PIN)
#if defined  (__IOC_V2__)
			GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
#else
			GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
#endif
			GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
			GPIO_InitStructure.GPIO_Pin = _SYS_SHG_PIN;
			GPIO_Init(_SYS_SHG_PORT, &GPIO_InitStructure);
			_SYS_SHG_DISABLE;
#endif
			timeout=0;
			key = temp = GPIO_ReadInputData(_FSW_PORT) & (_FSW0 | _FSW1 | _FSW2);
}
/*******************************************************************************
* Function Name	: Poll()
* Description		:	footswitch port polling
* Output				: int
* Return				: footswitch code, 20ms filter, on valid change
*******************************************************************************/
int   _GPIO::Poll(void) {
int		i=GPIO_ReadInputDataBit(_FSW_PORT,_FSW2);
			i=(i<<1) | GPIO_ReadInputDataBit(_FSW_PORT,_FSW1);
			i=(i<<1) | GPIO_ReadInputDataBit(_FSW_PORT,_FSW0);
			if(i != temp) {
				temp = i;
				timeout = __time__ + 5;
			} else 
					if(timeout && __time__ > timeout) {
						timeout=0;
						if(temp != key) {
							key=temp;
							return key << 13;
						}
			}
			return EOF;
}
/**
* @}
*/
