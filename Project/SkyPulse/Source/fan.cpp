/**
  ******************************************************************************
  * @file    dac.cpp
  * @author  Fotona d.d.
  * @version V1
  * @date    30-Sept-2013
  * @brief	 DA & DMA converters initialization
  *
  */ 
/** @addtogroup
* @{
*/
#include	"fan.h"
#include	"limits.h"
#include	"math.h"
#include	"ioc.h"
/*******************************************************************************
* Function Name	:
* Description		: 
* Output				:
* Return				: None
*******************************************************************************/
#ifdef __IOC_V2__	
_FAN::_FAN() :_TIM9() {
#else
_FAN::_FAN() :_TIM3(1) {
#endif
				fpl=20;
				fph=95;
				ftl=25;
				fth=40;
				timeout=__time__ + _FAN_ERR_DELAY;
				tacho=tacho_limit=0;
				idx=0;
}
/*******************************************************************************
* Function Name	:
* Description		: 
* Output				:
* Return				: None 2200
*******************************************************************************/
int			_FAN::Poll() {
int			e=_NOERR;
	
#ifndef __IOC_V2__	
				if(tacho && __time__ > timeout) {
					if(abs(tacho->Eval(Rpm()) - Tau()) > Tau()/10)
						e |= _fanTacho;
				}
				if(__time__ % (5*(Tau()/100)) == 0)
					_YELLOW2(20);
#else
				if(__time__ > timeout) {
					tacho=Tau1*600;					
					Tau1=0;
					timeout=__time__+100;
					if(tacho <= tacho_limit)
						e |= _fanTacho;
				}
#endif
				_fTIM->CCR1=_fTIM->ARR*Rpm()/100;
				return e;
}
/*******************************************************************************/
/**
	* @brief	:	Evaluates cooling water temp
	* @param	: None
	* @retval : (int) 100*T(C°)
	*/
/*******************************************************************************/
int			_FAN::Rpm(void) {
				return __ramp(Th2o(),ftl*100,fth*100,fpl,fph);
}
/*******************************************************************************/
/**
	* @brief	:	Loads the fan settings from ini file
	* @param	: None
	* @retval : None
	*/
void		_FAN::LoadSettings(FILE *f) {
char		c[128];
				fgets(c,sizeof(c),f);
				sscanf(c,"%d,%d,%d,%d,%d",&fpl,&fph,&ftl,&fth,&tacho_limit);
}
/*******************************************************************************/
/**
	* @brief	:	Appends the fan settings from ini file
	* @param	: None
	* @retval : None
	*/
void		_FAN::SaveSettings(FILE *f) {
				fprintf(f,"%5d,%5d,%5d,%5d,%5d           /.. fan\r\n",fpl,fph,ftl,fth,tacho_limit);
}
/*******************************************************************************/
/**
	* @brief	:	Prints the fan setup line on currently active terminal console  
	* @param	: left/rignt(a), up/down increment value (1), otherwise 0
	* @retval : None
	*/
void		_FAN::Increment(int a, int b) {
				idx= __min(__max(idx+b,0),4);

				if(a)
					timeout=__time__ + _FAN_ERR_DELAY;

				switch(idx) {
					case 1:
						fpl= __min(__max(fpl+a,5),fph);
						break;
					case 2:
						fph= __min(__max(fph+a,fpl),95);
						break;
					case 3:
						ftl= __min(__max(ftl+a,0),fth);
						break;
					case 4:
						fth= __min(__max(fth+a,ftl),50);
						break;
					}

				printf("\r:fan   %3d%c,%4.1lf'C",Rpm(),'%',(double)Th2o()/100);
				if(idx>0)
					printf("        %2d%c-%2d%c,%2d'-%2d'",fpl,'%',fph,'%',ftl,fth);
				for(int i=4*(5-idx)-1;idx && i--;printf("\b"));
}
/*******************************************************************************/
/**
	* @brief	TIM3 IC2 ISR
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
void		_FAN::Increment(int key)	{
						if(tacho_limit) {
							tacho_limit=0;
						printf("\r\n: tacho limit reset.... \r\n:");
						}	else {
							tacho_limit=tacho/2;
							printf("\r\n: flow limit set to %3.1lf rpm\r\n:",(double)tacho_limit);
						}
}
/**
* @}
*/ 

