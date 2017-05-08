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
/*******************************************************************************/
/**
	* @brief	TIM3 IC2 ISR
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
_FAN::_FAN() :_TIM3(1) {
				fpl=20;
				fph=95;
				ftl=25;
				fth=40;
				tacho = NULL;
				timeout=__time__ + 3000;
				idx=0;
}
/*******************************************************************************
* Function Name	:
* Description		: 
* Output				:
* Return				: None
*******************************************************************************/
int			_FAN::Poll() {
int			e=0;
				if(timeout==INT_MAX)
					TIM4->CCR1=TIM4->ARR;
				else {
					TIM4->CCR1=(int)((TIM4->ARR*__ramp(Th2o(),ftl*100,fth*100,fpl,fph))/100);
					if(tacho && __time__ > timeout) {
						if(abs(tacho->Eval(Rpm()) - Tau()) > Tau()/10)
							_SET_BIT(e,fanTacho);
					}
					if(__time__ % (5*(Tau()/100)) == 0)
						_YELLOW2(20);
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
int			_FAN::Rpm(void) {
				return __ramp(Th2o(),ftl*100,fth*100,fpl,fph);
}
/*******************************************************************************/
/**
	* @brief	TIM3 IC2 ISR
	* @param	: None
	* @retval : None
	*/
void		_FAN::LoadSettings(FILE *f) {
char		c[128];
				fgets(c,sizeof(c),f);
				sscanf(c,"%d,%d,%d,%d",&fpl,&fph,&ftl,&fth);
}
/*******************************************************************************/
/**
	* @brief	TIM3 IC2 ISR
	* @param	: None
	* @retval : None
	*/
void		_FAN::SaveSettings(FILE *f) {
				fprintf(f,"%5d,%5d,%5d,%5d                 /.. fan\r\n",fpl,fph,ftl,fth);
}
/*******************************************************************************/
/**
	* @brief	TIM3 IC2 ISR
	* @param	: None
	* @retval : None
	*/
void		_FAN::LoadLimits(FILE *f) {
char		c[128];
				tacho = new _FIT(4,FIT_POW);
	
				fgets(c,sizeof(c),f);
				sscanf(c,"%lf,%lf,%lf,%lf",&tacho->rp[0],&tacho->rp[1],&tacho->rp[2],&tacho->rp[3]);
}
/*******************************************************************************/
/**
	* @brief	TIM3 IC2 ISR
	* @param	: None
	* @retval : None
	*/
void		_FAN::SaveLimits(FILE *f) {
				fprintf(f,"%lf,%lf,%lf,%lf\r\n",tacho->rp[0],		tacho->rp[1],		tacho->rp[2],		tacho->rp[3]);
}
/*******************************************************************************/
/**
	* @brief	TIM3 IC2 ISR
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
int			_FAN::Increment(int a, int b) {
				idx= __min(__max(idx+b,0),4);

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
				
				printf("\r:fan       %5d%c,%4.1lf'C",Rpm(),'%',(double)Th2o()/100);
				if(idx>0)
					printf("        %2d%c-%2d%c,%2d'C-%2d'C",fpl,'%',fph,'%',ftl,fth);		
				for(int i=4*(5-idx);idx && i--;printf("\b"));
				return Rpm();
}
/*******************************************************************************/
/**
	* @brief	TIM3 IC2 ISR
	* @param	: None
	* @retval : None
	*/
bool		_FAN::Align(void) {
_FIT		*t;
	
int			_fpl=fpl,
				_fph=fph;

				if(tacho) {
					t=tacho; 
					tacho=NULL;
				} else {
					t = new _FIT(4,FIT_POW);
				}
				printf("\rfan  ");
				for(int i=_fpl; i<_fph; i+=10) {
					fpl=fph=i;
					_wait(3000,_thread_loop);
					t->Sample(i,Tau());
					printf(".");
				}
				for(int i=_fph; i>_fpl; i-=10) {
					fpl=fph=i;
					_wait(3000,_thread_loop);
					t->Sample(i,Tau());
					printf("\b \b");
				}
				
				fpl=_fpl;
				fph=_fph;
				
				if(!t->Compute())
					return false;
				else
					tacho=t;
					return true;
			}		
/*******************************************************************************/
/**
	* @brief	TIM3 IC2 ISR
	* @param	: None
	* @retval : None
	*/
bool		_FAN::Test(void) {
int			_fpl=fpl,
				_fph=fph;
_FIT		*t;

				if(!tacho)
					return false;
				t=tacho;
				tacho=NULL;

				do {
				for(int i=_fpl; i<_fph; ++i) {
					fpl=fph=i;
					_wait(200,_thread_loop);
					printf("\r\n%4.3lf,%4.3lf",
																					t->Eval(Rpm()),(double)Tau());				
				}
				for(int i=_fph; i>_fpl; --i) {
					fpl=fph=i;
					fpl=fph=i;
					_wait(200,_thread_loop);
					printf("\r\n%4.3lf,%4.3lf",
																					t->Eval(Rpm()),(double)Tau());				
				}
			} while(getchar() == EOF);
				printf("\r\n:");
				fpl=_fpl;
				fph=_fph;
				tacho=t;
				return true;
			}/**
* @}
*/ 

