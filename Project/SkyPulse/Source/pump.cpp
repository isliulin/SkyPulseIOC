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
#include	"isr.h"
#include	"math.h"
#include	"limits.h"
/*******************************************************************************/
/**
	* @brief	TIM3 IC2 ISR
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
_PUMP::_PUMP() :_TIM3(0)  {
				fpl=20;
				fph=50;
				ftl=25;
				fth=40;

				timeout=__time__ + 3000;

				tacho = pressure = current = NULL;
				offset.cooler=12500;
				gain.cooler=13300;
				idx=0;
}
/*******************************************************************************/
/**
	* @brief	TIM3 IC2 ISR
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
int			_PUMP::Poll(void) {
int			e=0;
				if(timeout==INT_MAX)
					DAC_SetChannel1Data(DAC_Align_12b_R,0);
				else {
					DAC_SetChannel1Data(DAC_Align_12b_R,__ramp(Th2o(),ftl*100,fth*100,fpl*0xfff/100,fph*0xfff/100));
					if(tacho && pressure && current && __time__ > timeout) {
						if(abs(tacho->Eval(Rpm()) - Tau()) > Tau()/10)    
							_SET_BIT(e,pumpTacho);
						if(abs(pressure->Eval(Rpm()) - adf.cooler) > adf.cooler/10)
							_SET_BIT(e,pumpPressure);
						if(abs(current->Eval(Rpm()) - adf.Ipump) > adf.Ipump/10)
							_SET_BIT(e,pumpCurrent);
					}
					if(__time__ % (5*(Tau()/100)) == 0)
						_BLUE2(20);
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
				sscanf(c,"%d,%d,%d,%d",&fpl,&fph,&ftl,&fth);
}
/*******************************************************************************/
/**
	* @brief	TIM3 IC2 ISR
	* @param	: None
	* @retval : None
	*/
void		_PUMP::SaveSettings(FILE *f) {
				fprintf(f,"%5d,%5d,%5d,%5d                 /.. pump\r\n",fpl,fph,ftl,fth);
}
/*******************************************************************************/
/**
	* @brief	TIM3 IC2 ISR
	* @param	: None
	* @retval : None
	*/
void		_PUMP::Enable() {
				if(timeout == INT_MAX)
					timeout=__time__ +  1000;
}
/*******************************************************************************/
/**
	* @brief	TIM3 IC2 ISR
	* @param	: None
	* @retval : None
	*/
void		_PUMP::Disable() {
				timeout=INT_MAX;
}
/*******************************************************************************/
/**
	* @brief	TIM3 IC2 ISR
	* @param	: None
	* @retval : None
	*/
void		_PUMP::LoadLimits(FILE *f) {
char		c[128];
				tacho = new _FIT(4,FIT_POW);
				pressure = new _FIT(4,FIT_POW);
				current = new _FIT(4,FIT_POW);
	
				fgets(c,sizeof(c),f);
				sscanf(c,"%lf,%lf,%lf,%lf",&tacho->rp[0],&tacho->rp[1],&tacho->rp[2],&tacho->rp[3]);
				fgets(c,sizeof(c),f);
				sscanf(c,"%lf,%lf,%lf,%lf",&pressure->rp[0],&pressure->rp[1],&pressure->rp[2],&pressure->rp[3]);
				fgets(c,sizeof(c),f);
				sscanf(c,"%lf,%lf,%lf,%lf",&current->rp[0],&current->rp[1],&current->rp[2],&current->rp[3]);
}
/*******************************************************************************/
/**
	* @brief	TIM3 IC2 ISR
	* @param	: None
	* @retval : None
	*/
void		_PUMP::SaveLimits(FILE *f) {
				fprintf(f,"%lf,%lf,%lf,%lf\r\n",tacho->rp[0],		tacho->rp[1],		tacho->rp[2],		tacho->rp[3]);
				fprintf(f,"%lf,%lf,%lf,%lf\r\n",pressure->rp[0],pressure->rp[1],pressure->rp[2],pressure->rp[3]);
				fprintf(f,"%lf,%lf,%lf,%lf\r\n",current->rp[0],	current->rp[1],	current->rp[2],	current->rp[3]);
}
/*******************************************************************************/
/**
	* @brief	TIM3 IC2 ISR
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
int			_PUMP::Increment(int a, int b)	{
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

				printf("\r:pump      %5d%c,%4.1lf'C,%4.1lf",Rpm(),'%',(double)Th2o()/100,(double)(adf.cooler-offset.cooler)/gain.cooler);
				if(idx>0)
					printf("   %2d%c-%2d%c,%2d'C-%2d'C,%4.3lf",fpl,'%',fph,'%',ftl,fth,(double)adf.Ipump/4096.0*3.3/2.1/16);		
				for(int i=4*(5-idx)+6;idx && i--;printf("\b"));
				return Rpm();
}
/*******************************************************************************/
/**
	* @brief	TIM3 IC2 ISR
	* @param	: None
	* @retval : None
	*/
bool		_PUMP::Align(void) {
_FIT		*t,*p,*c;
	
int			_fpl=fpl,
				_fph=fph;

				if(tacho && pressure && current) {
					t=tacho; 
					p=pressure; 
					c=current;
					tacho=pressure=current=NULL;
				} else {
					t = new _FIT(4,FIT_POW);
					p = new _FIT(4,FIT_POW);
					c = new _FIT(4,FIT_POW);	
				}
				printf("\rpump ");
				for(int i=_fpl; i<_fph; i+=5) {
					fpl=fph=i;
					_wait(1000,_thread_loop);
					t->Sample(i,Tau());
					p->Sample(i,(double)adf.cooler);
					c->Sample(i,(double)adf.Ipump);
					printf(".");
				}
				for(int i=_fph; i>_fpl; i-=5) {
					fpl=fph=i;
					_wait(1000,_thread_loop);
					t->Sample(i,Tau());
					p->Sample(i,(double)adf.cooler);
					c->Sample(i,(double)adf.Ipump);
					printf("\b \b");
				}
				
				fpl=_fpl;
				fph=_fph;
				
				if(!t->Compute())
					return false;
				else if(!p->Compute())
					return false;
				else if(!c->Compute())
					return false;
				else
					tacho=t;
					pressure=p;
					current=c;
					return true;
			}		
/*******************************************************************************/
/**
	* @brief	TIM3 IC2 ISR
	* @param	: None
	* @retval : None
	*/
bool		_PUMP::Test(void) {
int			_fpl=fpl,
				_fph=fph;
_FIT		*t,*p,*c;

				if(!tacho || !pressure || !current)
					return false;
				t=tacho;
				p=pressure;
				c=current;
				tacho=pressure=current=NULL;

				do {
				for(int i=_fpl; i<_fph; i+=5) {
					fpl=fph=i;
					_wait(1000,_thread_loop);
					printf("\r\n%4.3lf,%4.3lf,%4.3lf,%4.3lf,%4.3lf,%4.3lf",
																					t->Eval(Rpm()),(double)Tau(),
																					p->Eval(Rpm()),(double)(adf.cooler),
																					c->Eval(Rpm()),(double)(adf.Ipump));				
				}
				
				for(int i=_fph; i>_fpl; i-=5) {
					fpl=fph=i;
					fpl=fph=i;
					_wait(1000,_thread_loop);
					printf("\r\n%4.3lf,%4.3lf,%4.3lf,%4.3lf,%4.3lf,%4.3lf",
																					t->Eval(Rpm()),(double)Tau(),
																					p->Eval(Rpm()),(double)(adf.cooler),
																					c->Eval(Rpm()),(double)(adf.Ipump));				
				}
			} while(getchar() == EOF);
				printf("\r\n:");
				fpl=_fpl;
				fph=_fph;
				tacho=t;
				pressure=p;
				current=c;
				
				return true;
			}
/**
* @}
*/ 


