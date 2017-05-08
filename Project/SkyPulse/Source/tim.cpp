/**
	******************************************************************************
	* @file		tim.cpp
	* @author	Fotona d.d.
	* @version
	* @date		
	* @brief	Timers initialization & ISR
	*
	*/	
/** @addtogroup
* @{
*/
#include "tim.h"
#include <string.h>
#include <math.h>
#include <isr.h>

_TIM  *me=NULL;
//short _TIM::speaker[2*MAX_NGRAN*MAX_NCHAN*MAX_NSAMP];
short _TIM::speaker[1000];
/*******************************************************************************/
/**
	* @brief	TIM3 IC2 ISR
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
_TIM*	_TIM::Instance() {
	if(me==NULL)
		me=new _TIM();
	return me;
}
/*******************************************************************************
* Function Name	: Timer_Init
* Description		: Configure timer pins as output open drain
* Output				 : TIM8
* Return				 : None
*******************************************************************************/
_TIM::_TIM() {
TIM_TimeBaseInitTypeDef		TIM_TimeBaseStructure;
TIM_OCInitTypeDef					TIM_OCInitStructure;

#ifndef __SIMULATION__
GPIO_InitTypeDef					GPIO_InitStructure;
// ________________________________________________________________________________
// TIM1-TIM8, IGBT pwm outputs

			GPIO_StructInit(&GPIO_InitStructure);
			GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
			GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;

			GPIO_PinAFConfig(GPIOE, GPIO_PinSource9,	GPIO_AF_TIM1);
			GPIO_PinAFConfig(GPIOE, GPIO_PinSource11, GPIO_AF_TIM1);
			GPIO_PinAFConfig(GPIOE, GPIO_PinSource13, GPIO_AF_TIM1);
			GPIO_PinAFConfig(GPIOE, GPIO_PinSource14, GPIO_AF_TIM1);
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 |	GPIO_Pin_11 |GPIO_Pin_13 |	GPIO_Pin_14 ;
			GPIO_Init(GPIOE, &GPIO_InitStructure);

			GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_TIM8);
			GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, GPIO_AF_TIM8);
			GPIO_PinAFConfig(GPIOC, GPIO_PinSource8, GPIO_AF_TIM8);
			GPIO_PinAFConfig(GPIOC, GPIO_PinSource9, GPIO_AF_TIM8);
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 |	GPIO_Pin_7 |GPIO_Pin_8 |	GPIO_Pin_9 ;
			GPIO_Init(GPIOC, &GPIO_InitStructure);

// TIM4, fan pwm output	
			GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
			GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
			
			GPIO_PinAFConfig(GPIOD, GPIO_PinSource12, GPIO_AF_TIM4);
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
			GPIO_Init(GPIOD, &GPIO_InitStructure);
// ________________________________________________________________________________
// TIMebase setup
#endif
			TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
			TIM_OCStructInit(&TIM_OCInitStructure);

			TIM_TimeBaseStructure.TIM_Prescaler = SystemCoreClock/4000000-1;
			TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_CenterAligned1;
			TIM_TimeBaseStructure.TIM_RepetitionCounter=1;

// TIM 1,8

			TIM_TimeBaseStructure.TIM_Period = _PWM_RATE;
			TIM_DeInit(TIM1);
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
			TIM_TimeBaseInit(TIM1,&TIM_TimeBaseStructure);
			TIM_DeInit(TIM8);
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE);
			TIM_TimeBaseInit(TIM8,&TIM_TimeBaseStructure);
			
// TIM 4 
			TIM_DeInit(TIM4);
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
			
			TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
			TIM_TimeBaseStructure.TIM_Prescaler = 0;
			TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
			TIM_TimeBaseStructure.TIM_Period = SystemCoreClock/2/25000;

			TIM_TimeBaseInit(TIM4,&TIM_TimeBaseStructure);
// ________________________________________________________________________________
// Output Compares	TIM1,8
			TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
			TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
			TIM_OCInitStructure.TIM_Pulse=0;
			TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

			TIM_OC1Init(TIM1, &TIM_OCInitStructure);
			TIM_OC1Init(TIM8, &TIM_OCInitStructure);
			TIM_OC3Init(TIM1, &TIM_OCInitStructure);
			TIM_OC3Init(TIM8, &TIM_OCInitStructure);

			TIM_OC2Init(TIM1, &TIM_OCInitStructure);
			TIM_OC2Init(TIM8, &TIM_OCInitStructure);
			TIM_OC4Init(TIM1, &TIM_OCInitStructure);
			TIM_OC4Init(TIM8, &TIM_OCInitStructure);
//_________________________________________________________________________________
// Output Compares	TIM4
			TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
			TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
			TIM_OCInitStructure.TIM_Pulse=0;
			TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
			TIM_OC1Init(TIM4, &TIM_OCInitStructure);
// ________________________________________________________________________________
// Startup
			TIM_CtrlPWMOutputs(TIM1, ENABLE);
			TIM_CtrlPWMOutputs(TIM8, ENABLE);
			TIM_CtrlPWMOutputs(TIM4, ENABLE);

			TIM_Cmd(TIM1,ENABLE);
			TIM_Cmd(TIM8,ENABLE);
			TIM_Cmd(TIM4,ENABLE);
				
			memset(t1, 0, sizeof(t1)); 
			memset(t2, 0, sizeof(t2)); 
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
int		_TIM::Pwm(int tim, int value ) {
			switch(tim) {
				case 0: TIM1->CCR1=value; break;
				case 1: TIM1->CCR2=value; break;
				case 2: TIM1->CCR3=value; break;
				case 3: TIM1->CCR4=value; break;
				case 4: TIM8->CCR1=value; break;
				case 5: TIM8->CCR2=value; break;
				case 6: TIM8->CCR3=value; break;
				case 7: TIM8->CCR4=value; break;
				default:return -1;
			}
			return value;
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
int		_TIM::Pwm(int tim) {
			switch(tim) {
				case 0: return TIM1->CCR1;
				case 1: return TIM1->CCR2;
				case 2: return TIM1->CCR3;
				case 3: return TIM1->CCR4;
				case 4: return TIM8->CCR1;
				case 5: return TIM8->CCR2;
				case 6: return TIM8->CCR3;
				case 7: return TIM8->CCR4;
				default:return EOF;
			}
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
bool	_TIM::Busy(int tim) {
			if(t1[tim] || t2[tim])
				return true;
			else
				return false;
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
int		_TIM::Pwm(int tim, int value, int i, int j ) {
			Pwm(tim, value);
			t1[tim]=__time__ + i;
			t2[tim]=__time__ + j;
			return Pwm(tim);
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void	_TIM::Poll(void) {
			for(int n=0; n<8; ++n) { 
				if(t1[n] && __time__ >= t1[n]) {
					t1[n]=0;
					if(Pwm(n) == _PWon)									// striktno samo na ti dve vrednosti
						Pwm(n,_PWoff);										// da preskoci prop. ventil !!!
					else
						if(Pwm(n) == _PWoff)
							Pwm(n,_PWon);
				}
				if(t2[n] && __time__ >= t2[n]) {
					t2[n]=0;
				}
			}		
}
/*******************************************************************************
* Function Name	:
* Description		:	TIM3 pump && fan tacho IC
* Output				:
* Return				:
*******************************************************************************/
static _TIM3 *Instance[]={NULL,NULL,NULL,NULL};
/*******************************************************************************
*/
_TIM3::_TIM3(int n) {

GPIO_InitTypeDef GPIO_InitStructure;
TIM_TimeBaseInitTypeDef		TIM_TimeBaseStructure;
TIM_ICInitTypeDef					TIM_ICInitStructure;
//
// if called first time ???
		if(!Instance[0] && !Instance[1] && !Instance[2] && !Instance[3]) {
// gpio		
			GPIO_StructInit(&GPIO_InitStructure);
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
			
			GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_TIM3);
			GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_TIM3);
			
			GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
			GPIO_Init(GPIOA, &GPIO_InitStructure);
// timebase
			TIM_DeInit(TIM3);
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
			TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
			TIM_ICStructInit(&TIM_ICInitStructure);
			TIM_TimeBaseStructure.TIM_Prescaler = 60;
			TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
			TIM_TimeBaseStructure.TIM_Period = 0xffff;
			TIM_TimeBaseInit(TIM3,&TIM_TimeBaseStructure);
// input Compares
			TIM_ICStructInit(&TIM_ICInitStructure);	
			TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_BothEdge;
			TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
			TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
			TIM_ICInitStructure.TIM_ICFilter = 0x000f;
			TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;
			TIM_ICInit(TIM3, &TIM_ICInitStructure);
			TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;
			TIM_ICInit(TIM3, &TIM_ICInitStructure);
			
			TIM_Cmd(TIM3,ENABLE);
			TIM_ITConfig(TIM3, TIM_IT_CC1,ENABLE);
			TIM_ITConfig(TIM3, TIM_IT_CC2,ENABLE);
			NVIC_EnableIRQ(TIM3_IRQn);
		}
		
		Instance[n]=this;
		to=timeout=0;
		tauN=32;
		while(tauN--)
			tau[tauN]=0;
}
/*******************************************************************************
* Function Name	:
* Description		:	TIM3 pump && fan tacho IC
* Output				:
* Return				:
*******************************************************************************/
_TIM3::~_TIM3() {
		NVIC_DisableIRQ(TIM3_IRQn);
}
/*******************************************************************************
* Function Name	: ISR
* Description		:	TIM3 input capture interrupt service
* Input					: capture register value
* Return				: None, tau parameter set to dt value, usecs
********************************************************************************/
void	_TIM3::ISR(int t) {
			to=t-to;
			if(to < 0)
				to += (1<<16);
			if(to > 100) {
int			a=tau[tauN];
				tau[tauN]=to;
				tauN=++tauN % 32;
				tau[tauN]=a-tau[tauN]+to;				
			}
			to=t;
			timeout=__time__ + 50;
}
/*******************************************************************************
* Function Name	: ISR
* Description		:	TIM3 input capture interrupt service
* Input					: capture register value
* Return				: None, tau parameter set to dt value, usecs
*******************************************************************************/
int		_TIM3::Tau(void) {
			NVIC_DisableIRQ(TIM3_IRQn);
int		i=tau[tauN]/32;
			NVIC_EnableIRQ(TIM3_IRQn);

			if(__time__ > timeout)
				return EOF;
			else
				return i;
}
/*******************************************************************************
* Function Name	:
* Description		:	TIM3 pump && fan tacho IC
* Output				:
* Return				:
*******************************************************************************/
extern	"C" {
void	TIM3_IRQHandler(void) {
			if(TIM_GetITStatus(TIM3,TIM_IT_CC1)==SET) {
				Instance[0]->ISR(TIM_GetCapture1(TIM3));
				TIM_ClearITPendingBit(TIM3, TIM_IT_CC1);
			}
			if(TIM_GetITStatus(TIM3,TIM_IT_CC2)==SET) {
				Instance[1]->ISR(TIM_GetCapture2(TIM3));
				TIM_ClearITPendingBit(TIM3, TIM_IT_CC2);
			}
			if(TIM_GetITStatus(TIM3,TIM_IT_CC3)==SET) {
				Instance[2]->ISR(TIM_GetCapture3(TIM3));
				TIM_ClearITPendingBit(TIM3, TIM_IT_CC3);
			}
			if(TIM_GetITStatus(TIM3,TIM_IT_CC4)==SET) {
				Instance[3]->ISR(TIM_GetCapture4(TIM3));
				TIM_ClearITPendingBit(TIM3, TIM_IT_CC4);
			}
}
}
/**
* @}
*/ 

