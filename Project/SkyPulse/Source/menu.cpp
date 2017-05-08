// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "limits.h"
#include "stdlib.h"
#include "menu.h"
#include "lcd.h"

/** @addtogroup
* @{
*/
/**
* @brief  ADC	common init
* @param  None
* @retval None
*/
/*******************************************************************************
* Function Name	:
* Description		:
* Output				:
* Return				: None
*******************************************************************************/
_MENU::_MENU(char *c, _MENU *m0, _MENU *m1, _MENU *m2, _MENU *m3) {
	str = c;
	item = EOF;
	next[0] = m0;
	next[1] = m1;
	next[2] = m2;
	next[3] = m3;
	f = NULL;
}
/*******************************************************************************
* Function Name	:
* Description		:
* Output				:
* Return				: None
*******************************************************************************/
_MENU::_MENU(int(*fp)(void *), void *ap) {
	item = EOF;
	str = NULL;
	f = fp;
	arg = ap;
	next[0] = next[1] = next[2] = next[3] = NULL;
}
/*******************************************************************************
* Function Name	:
* Description		:
* Output				:
* Return				: None
*******************************************************************************/
void _MENU::Refresh() {
	if (item < 0) {
		if (str) {
			
#ifdef USE_LCD
			LCD_SetFont(&Font8x12);
			sFONT *f = LCD_GetFont();
			LCD_SetTextColor(LCD_COLOR_GREY);
			LCD_DisplayStringLine(LCD_PIXEL_HEIGHT - 5 * f->Height / 2, (uint8_t *)str);
			for (int i = 0; i<4; ++i)
				LCD_DrawRect((9 * i + 1)*f->Width,
				LCD_PIXEL_HEIGHT - 3 * f->Height,
				9 * f->Width,
				2 * f->Height);
#endif		
//			printf("%s\r", str);
		}
	}
	else
		next[item]->Refresh();
}
/*******************************************************************************
* Function Name	:
* Description		:
* Output				:
* Return				: None
*******************************************************************************/
int	 _MENU::Poll(int selected) {
	if(item != EOF) {
		selected = next[item]->Poll(selected);
	} else {
		item = selected;
		if (next[item]) {
			if (next[item]->f)
				selected = next[item]->f(next[item]->arg);
			else if (next[item]->str)
				selected = EOF;
			else
				selected = INT_MAX;
		} else
			selected = INT_MAX;
	}
	if (selected == EOF)
		return selected;
	else {
		item = EOF;
		return --selected;
	}
}

/**
* @}
*/

