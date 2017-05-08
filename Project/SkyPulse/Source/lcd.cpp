/**
  ******************************************************************************
  * @file    lcd.cpp
  * @author  Fotona d.d.
  * @version V1
  * @date    30-Sept-2013
  * @brief	 DA & DMA converters initialization
  *
  */ 
/** @addtogroup
* @{
*/

#include	"lcd.h"
#include	"isr.h"
#include	"limits.h"
/*******************************************************************************
* Function Name	:
* Description		: 
* Output				:
* Return				: None
*******************************************************************************/
_LCD::_LCD() {
#ifdef USE_LCD
			STM32f4_Discovery_LCD_Init();
			LCD_SetBackColor(LCD_COLOR_BLACK);
			LCD_SetTextColor(LCD_COLOR_YELLOW);
			LCD_SetFont(&Font8x12);
			Grid();
#endif
}
/*******************************************************************************
* Function Name	:
* Description		: 
* Output				:
* Return				: None
*******************************************************************************/
void	_LCD::Home() {
			x=y=0;
			}
/*******************************************************************************
* Function Name	:
* Description		: 
* Output				:
* Return				: None
*******************************************************************************/		
void	_LCD::Grid() {
			LCD_Clear(LCD_COLOR_BLACK);
			LCD_SetTextColor(0x1ce7);
			for(int i=0; i<LCD_PIXEL_WIDTH; i+=50)
				LCD_DrawLine(i,0,LCD_PIXEL_WIDTH,LCD_DIR_VERTICAL);
			for(int i=0; i<LCD_PIXEL_HEIGHT/2; i+=50) {
				LCD_DrawLine(0,LCD_PIXEL_HEIGHT/2+i,LCD_PIXEL_WIDTH,LCD_DIR_HORIZONTAL);
				LCD_DrawLine(0,LCD_PIXEL_HEIGHT/2-i,LCD_PIXEL_WIDTH,LCD_DIR_HORIZONTAL);
			}
}
/**
* @}
*/ 
