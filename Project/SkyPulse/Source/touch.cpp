// ConsoleApplication1.cpp : Defines the entry point for the console application.
//
#include "touch.h"
#include "isr.h"
#include "stmpe811qtr.h"
#include "stm32f4_discovery_lcd.h"
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
/**
* @brief  ADC	common init
* @param  None
* @retval None
*/
/*******************************************************************************
* Function Name			:
* Description			:
* Output				:
* Return				: None
*******************************************************************************/
_TOUCH::_TOUCH() {
		n=x=y=t=0;	
		IOE_Config();
}
/*******************************************************************************
* Function Name			:
* Description			:
* Output				:
* Return				: None
*******************************************************************************/
int	 _TOUCH::Poll(void) {
		TS_STATE	*ts = IOE_TS_GetState();
		if (ts->TouchDetected) {
			x += ts->X;
			y += ts->Y;
			++n;
			t=__time__ + 10;
		}
				
		if ((__time__ > t && n > 5) || (n > 50)) {
			int retx = 3- (x/n)/1000;
			int rety = (y/n-3450)/100;
			n = x = y = 0;
			if(retx >= 0 && retx < 4 && !rety)
				return retx;
		}
		return EOF;
}

/**
* @}
*/

