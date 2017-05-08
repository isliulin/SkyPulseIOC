/**
  ******************************************************************************
  * @file    pilot.cpp
  * @author  Fotona d.d.
  * @version V1
  * @date    30-Sept-2013
  * @brief
  *
  */ 
/** @addtogroup
* @{
*/

#include	"pilot.h"
#include	"isr.h"
/*******************************************************************************/
/**
	* @brief
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
_PILOT::_PILOT() {
	Value=count=0;
	enabled=false;
		
}
/*******************************************************************************/
/**
	* @brief
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
_PILOT::~_PILOT() {
}
/*******************************************************************************/
/**
	* @brief
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
void	_PILOT::Poll(void) {
			if(enabled && count < Value)
				GPIO_ResetBits(GPIOD,GPIO_Pin_13);
			else
				GPIO_SetBits(GPIOD,GPIO_Pin_13);
			count = (count + 5) % 100;
}
/*******************************************************************************/
/**
	* @brief
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
int		_PILOT::Increment(int updown, int leftright) {

			On();
			Value =__min(__max(0,Value+5*updown),100);	
			printf("\r:pilot       %3d%c",Value,'%');
			return Value;
}
/*******************************************************************************/
/**
	* @brief	TIM3 IC2 ISR
	* @param	: None
	* @retval : None
	*/
void	_PILOT::LoadSettings(FILE *f) {
char	c[128];
			fgets(c,sizeof(c),f);
			sscanf(c,"%d",&Value);
}
/*******************************************************************************/
/**
	* @brief	TIM3 IC2 ISR
	* @param	: None
	* @retval : None
	*/
void	_PILOT::SaveSettings(FILE *f) {
			fprintf(f,"%5d                                   /.. pilot\r\n",Value);
}
/**
* @}
*/ 

