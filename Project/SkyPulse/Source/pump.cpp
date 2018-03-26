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

#include	"pump.h"
#include	"fit.h"
#include	"term.h"
#include	"ioc.h"
#include	"math.h"
#include	"limits.h"
/*******************************************************************************/
/**
	* @brief	TIM3 IC2 ISR
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
#ifdef __IOC_V2__	
_PUMP::_PUMP() :_TIM9()  {
#else
_PUMP::_PUMP() :_TIM3(0)  {
#endif
				fpl=20;
				fph=50;
				ftl=25;
				fth=40;

				timeout=__time__ + _PUMP_ERR_DELAY;

				offset.cooler=_BAR(1);
				gain.cooler=_BAR(1);
				idx=0;
				mode=(1<<PUMP_FLOW);
				curr_limit=0;
				Enabled=true;
}
/*******************************************************************************/
/**
	* @brief	TIM3 IC2 ISR
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
int			_PUMP::Poll(void) {
int			e=_NOERR;
				if(Enabled) {
					if(DAC_GetDataOutputValue(0) < __ramp(Th2o(),ftl*100,fth*100,fpl*0xfff/100,fph*0xfff/100))
						DAC_SetChannel1Data(DAC_Align_12b_R,DAC_GetDataOutputValue(0) + 1);
					else
						DAC_SetChannel1Data(DAC_Align_12b_R,DAC_GetDataOutputValue(0) - 1);
				} else	
						DAC_SetChannel1Data(DAC_Align_12b_R,0);
				if(__time__ > timeout) {
					if(curr_limit && adf.Ipump > curr_limit)
						e |= _pumpCurrent;
					if(Tau2==0)
						e |= _flowTacho;
					Tau2=0;
					Flow=Tau2*600;					
				} 	

				return e;
}
/*******************************************************************************/
/**
	* @brief	TIM3 IC2 ISR
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
int			_PUMP::Rpm(void) {
				return __ramp(Th2o(),ftl*100,fth*100,fpl,fph);
}
/*******************************************************************************/
/**
	* @brief	TIM3 IC2 ISR
	* @param	: None
	* @retval : None
	*/
void		_PUMP::LoadSettings(FILE *f) {
char		c[128];
				fgets(c,sizeof(c),f);
				sscanf(c,"%d,%d,%d,%d,%d",&fpl,&fph,&ftl,&fth,&curr_limit);
}
/*******************************************************************************/
/**
	* @brief	TIM3 IC2 ISR
	* @param	: None
	* @retval : None
	*/
void		_PUMP::SaveSettings(FILE *f) {
				fprintf(f,"%5d,%5d,%5d,%5d,%5d           /.. pump\r\n",fpl,fph,ftl,fth,curr_limit);
}
/*******************************************************************************/
/**
	* @brief	TIM3 IC2 ISR
	* @param	: None
	* @retval : None
	*/
void		_PUMP::Enable() {
				if(Enabled==false) {
					timeout=__time__ +  _PUMP_ERR_DELAY;
					Enabled=true;
				}
}

/*******************************************************************************/
/**
	* @brief	TIM3 IC2 ISR
	* @param	: None
	* @retval : None
	*/
void		_PUMP::Disable() {
				Enabled=false;
}
/*******************************************************************************/
/**
	* @brief	TIM3 IC2 ISR
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
int			_PUMP::Increment(int a, int b)	{
				idx= __min(__max(idx+b,0),5);
	
				switch(idx) {
					case 0:
						if(a) {
							mode ^= (1<<PUMP_FLOW);
							if(mode & (1<<PUMP_FLOW))
								printf("\b\b\b\b lpm");
							else
								printf("\b\b\b\b bar");
							return Rpm();
						}
						break;
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
					case 5:
						if(a > 0) {
							curr_limit=adf.Ipump + adf.Ipump/4;
							printf("\r\n: current limit set to %4.3lfA\r\n:",(double)curr_limit/4096.0*3.3/2.1/16);
						}
						if(a < 0) {
							curr_limit=0;
							printf("\r\n: current reset.... \r\n:");
						}
						break;
				}
		
				if(mode & (1<<PUMP_FLOW))
					printf("\r:pump  %3d%c,%4.1lf'C,%4.1lf",Rpm(),'%',(double)Th2o()/100,(double)Flow/22000);
				else
					printf("\r:pump  %3d%c,%4.1lf'C,%4.1lf",Rpm(),'%',(double)Th2o()/100,(double)(adf.cooler-offset.cooler)/gain.cooler);

				if(idx>0)
					printf("   %2d%c-%2d%c,%2d'-%2d',%4.3lf",fpl,'%',fph,'%',ftl,fth,(double)adf.Ipump/4096.0*3.3/2.1/16);		
				for(int i=4*(5-idx)+5;idx && i--;printf("\b"));
				return Rpm();
}
/**
* @}
*/ 


