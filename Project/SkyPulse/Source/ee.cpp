/**
	******************************************************************************
	* @file		pyro.cpp
	* @author	Fotona d.d.
	* @version
	* @date
	* @brief	thermopile sensor class
	*
	*/
	
/** @addtogroup
* @{
*/

#include	"ee.h"
#include	"stdio.h"
#include	"string.h"

static		_EE*	me=NULL;
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
_EE::_EE() {	
			status=_IDLE;
			phase=nbits=0;
			ISR(this);
	
GPIO_InitTypeDef					
			GPIO_InitStructure;
			GPIO_StructInit(&GPIO_InitStructure);
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
			GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
			GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
			GPIO_InitStructure.GPIO_Pin = EE_BIT;
			GPIO_Init(EE_PORT, &GPIO_InitStructure);
			GPIO_SetBits(EE_PORT,EE_BIT);

			TIM_TimeBaseInitTypeDef TIM;
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5,ENABLE);
			TIM_TimeBaseStructInit(&TIM);
			TIM.TIM_Prescaler = (SystemCoreClock/2000000)-1;
			TIM.TIM_Period = _tRD;
			TIM.TIM_ClockDivision = 0;
			TIM.TIM_CounterMode = TIM_CounterMode_Up;
			TIM_TimeBaseInit(TIM5,&TIM);
			TIM_ARRPreloadConfig(TIM5,DISABLE);
			TIM_ITConfig(TIM5,TIM_IT_Update,ENABLE);
			NVIC_EnableIRQ(TIM5_IRQn);
}
/*******************************************************************************/
/**
	* @brief	TIM3 IC2 ISR
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
void	_EE::ISR(_EE *p) {
			if(p) {
				me=p;																										// klic za prijavo instance
			} else {																									// klic iz ISR, instanca in buffer morata bit ze formirana
				switch(status) {
					case _IDLE:
						GPIO_SetBits(EE_PORT,EE_BIT);
						TIM_Cmd(TIM5,DISABLE);
						break;

					case _BUSY:
						switch(phase++) {
							case 0:
								TIM5->ARR=5;
								GPIO_ResetBits(EE_PORT,EE_BIT);
								for(int i=0; i<10; ++i);
								if(temp & (1<<--nbits))
									GPIO_SetBits(EE_PORT,EE_BIT);
								for(int i=0; i<10; ++i);
								if(GPIO_ReadInputDataBit(EE_PORT,EE_BIT)==RESET)
									temp &= ~(1<<nbits);								
								break;
							case 1:
								TIM5->ARR=10;
								GPIO_SetBits(EE_PORT,EE_BIT);
								phase=0;
								if(!nbits)
									status=_IDLE;
								break;
						}
						break;

					case _RESET:
						switch(phase++) {
							case 0:
								GPIO_ResetBits(EE_PORT,EE_BIT);
								TIM5->ARR=_tRESET-1;
								break;
							case 1:
								GPIO_SetBits(EE_PORT,EE_BIT);
								TIM5->ARR=_tRRT-1;
								break;
							case 2:
								GPIO_ResetBits(EE_PORT,EE_BIT);
								for(int i=0; i<10; ++i);
								GPIO_SetBits(EE_PORT,EE_BIT);
								status=_IDLE;
								break;
						}
						break;
				}
			}
		}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
char	*_EE::getSerial(char *c) {
			sprintf(c,"b2-00-");
			Exchg(c);
			_wait(2,_thread_loop);
			sprintf(c,"b3-ff_ff_ff_ff_ff_ff_ff_ff-");
			Exchg(c);
			return c;
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
char	*_EE::getPage(int n, char *c) {
			sprintf(c,"a2-%02X-",8*n);
			Exchg(c);
			_wait(2,_thread_loop);
			sprintf(c,"a3-ff_ff_ff_ff_ff_ff_ff_ff-");
			return Exchg(c);
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
char	*_EE::putPage(int n, char *c) {
char	cc[128];
	
			sprintf(cc,"a2-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-",8*n,c[0],c[1],c[2],c[3],c[4],c[5],c[6],c[7]);
			Exchg(cc);
			strcpy(c,cc);
			return c;
}

/*

:b2-00-
:b3-ff_ff_ff_ff_ff_ff_ff_ff-

:a2-00-
:a3-ff_ff_ff_ff_ff_ff_ff_ff-
:a2-08-
:a3-ff_ff_ff_ff_ff_ff_ff_ff-
:a2-10-
:a3-ff_ff_ff_ff_ff_ff_ff_ff-
:a2-18-
:a3-ff_ff_ff_ff_ff_ff_ff_ff-


:a2-00-01-02-03-04-05-06-07-08-
:a2-08-11-12-13-14-15-16-17-18-
:a2-10-21-22-23-24-25-26-27-28-
:a2-18-31-32-33-34-35-36-37-38-

*/
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
char	*_EE::Exchg(char *p) {
int		i,j,t;
char	k,q[128];
		if(!*p) {
			status=_RESET;
			phase=0;
			TIM_Cmd(TIM5,ENABLE);
			return NULL;
		}
		for(j=0; sscanf(&p[j],"%02X%c",&i,&k) > 0; j+=3) {
			temp=i<<1;
			if(k=='-')
				temp |= 1;
			phase=0;
			nbits=9;
			status=_BUSY;
			t=__time__+5;
			TIM_Cmd(TIM5,ENABLE);
			while(status != _IDLE)
				if(__time__ > t)
					break;
			if(status == _IDLE) {
				if(temp % 2)
					sprintf(&q[j],"%02X-",temp>>1);
				else
					sprintf(&q[j],"%02X_",temp>>1);
			}	else {
				sprintf(&q[j],"---");
				break;
			}
		}
		strcpy(p,q);
		return p;
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
_EE::~_EE() {	
			TIM_ITConfig(TIM5,TIM_IT_Update,DISABLE);
			NVIC_DisableIRQ(TIM5_IRQn);
}

extern "C" {
/*******************************************************************************/
/**
	* @brief	TIM5_IRQHandler, klice staticni ISR handler, indikacija je NULL pointer,
	*					sicer pointer vsebuje parent class !!! Mora bit extern C zaradi overridanja 
						vektorjev v startupu
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
void	TIM5_IRQHandler(void) {
			if (TIM_GetITStatus(TIM5,TIM_IT_Update) != RESET) {
				TIM_ClearITPendingBit(TIM5,TIM_IT_Update);
				me->ISR(NULL);
				}
			}
}
/**
* @}
*/ 
