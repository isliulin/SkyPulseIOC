/**
	******************************************************************************
	* @file		timers.c
	* @author	Fotona d.d.
	* @version V1
	* @date		30-Sept-2013
	* @brief	Timers initialization & ISR
	*
	*/
	
/** @addtogroup PFM6_Setup
* @{
*/
#include	"pfm.h"
#include	<math.h>
_TIM18DMA	TIM18_buf[_MAX_BURST/(10*_uS)];
/*******************************************************************************
* Function Name	: Timer_Init
* Description		: Configure timer pins as output open drain
* Output				 : TIM1, TIM8
* Return				 : None
*******************************************************************************/
void 	Initialize_TIM() {
TIM_TimeBaseInitTypeDef		TIM_TimeBaseStructure;
TIM_OCInitTypeDef					TIM_OCInitStructure;
TIM_ICInitTypeDef					TIM_ICInitStructure;
DMA_InitTypeDef						DMA_InitStructure;
GPIO_InitTypeDef					GPIO_InitStructure;
EXTI_InitTypeDef   				EXTI_InitStructure;
// ________________________________________________________________________________
// GPIO setup
#ifdef __DISCO__
// ________________________________________________________________________________
// USB host power supply
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
		GPIO_Init(GPIOC, &GPIO_InitStructure);
		GPIO_SetBits(GPIOC,GPIO_Pin_0);
#endif
// TIM3, pwm out/tacho in PA6,PA7

		GPIO_StructInit(&GPIO_InitStructure);
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;

		GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_TIM3);
		GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_TIM3);

		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
// ________________________________________________________________________________
// TRIGGER 1, TRIGGER 2
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13;
		GPIO_Init(GPIOD, &GPIO_InitStructure);
		_TRIGGER1_OFF;
		_TRIGGER2_OFF;

//  CROWBAR _______________________________________________________________________
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;					
		GPIO_Init(GPIOD, &GPIO_InitStructure);
		
		SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOD, EXTI_PinSource14);
		EXTI_ClearITPendingBit(EXTI_Line14);
		EXTI_InitStructure.EXTI_Line = EXTI_Line14;
		EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
		EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;  
		EXTI_InitStructure.EXTI_LineCmd = ENABLE;
		EXTI_Init(&EXTI_InitStructure);
		
// 	PFM_FAULT_SENSE_______________________________________________________________
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;						
		GPIO_Init(GPIOE, &GPIO_InitStructure);
		
		SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource8);
		EXTI_ClearITPendingBit(EXTI_Line8);
		EXTI_InitStructure.EXTI_Line = EXTI_Line8;
		EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
		EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;  
		EXTI_InitStructure.EXTI_LineCmd = ENABLE;
		EXTI_Init(&EXTI_InitStructure);
// ________________________________________________________________________________
// TIM1, TIM8 IGBT pwm outputs

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

// DMA setup _____________________________________________________________________
		DMA_StructInit(&DMA_InitStructure);
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
		DMA_DeInit(DMA2_Stream5);
		DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)TIM18_buf;
		DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
		DMA_InitStructure.DMA_BufferSize =_MAX_BURST/_PWM_RATE_HI*5;	// 5 transferji v burstu (sic!)
		DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
		DMA_InitStructure.DMA_Priority = DMA_Priority_High;
		DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
		DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
		DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
		DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;
		DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
		DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
		DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;

// TIM1 ___________________________________________________________________________
		DMA_InitStructure.DMA_Channel = DMA_Channel_6;
		DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)TIM1_BASE + 0x4C;	//~~~
		DMA_Init(DMA2_Stream5, &DMA_InitStructure);

// TIM8 ___________________________________________________________________________
		DMA_InitStructure.DMA_Channel = DMA_Channel_7;
		DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)TIM8_BASE + 0x4C;
		DMA_Init(DMA2_Stream1, &DMA_InitStructure);

// ________________________________________________________________________________
// TIMebase setup
		TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
		TIM_OCStructInit(&TIM_OCInitStructure);

		TIM_TimeBaseStructure.TIM_Prescaler = 0;
		TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_CenterAligned1;
		TIM_TimeBaseStructure.TIM_RepetitionCounter=1;

// TIM 1,8

		TIM_TimeBaseStructure.TIM_Period = _PWM_RATE_HI;
		TIM_DeInit(TIM1);
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
		TIM_TimeBaseInit(TIM1,&TIM_TimeBaseStructure);
		TIM_DeInit(TIM8);
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE);
		TIM_TimeBaseInit(TIM8,&TIM_TimeBaseStructure);

// 90 deg. shift
		TIM_SetCounter(TIM1,0);
		TIM_SetCounter(TIM8,_PWM_RATE_HI/2);

// TIM3

		TIM_TimeBaseStructure.TIM_Period = _FAN_PWM_RATE/2;
 		TIM_DeInit(TIM3);
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
		TIM_TimeBaseInit(TIM3,&TIM_TimeBaseStructure);

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

		TIM_OCInitStructure.TIM_Pulse=_PWM_RATE_HI;
		TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;

		TIM_OC2Init(TIM1, &TIM_OCInitStructure);
		TIM_OC2Init(TIM8, &TIM_OCInitStructure);
		TIM_OC4Init(TIM1, &TIM_OCInitStructure);
		TIM_OC4Init(TIM8, &TIM_OCInitStructure);

// Output Compares, CH1	TIM3

		TIM_OCInitStructure.TIM_Pulse=1;
		TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
		TIM_OC1Init(TIM3, &TIM_OCInitStructure);

// Input Captures CH2 TIM3

		TIM_ICStructInit(&TIM_ICInitStructure);														// Input Capture channels
		TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Falling;			// Falling edge capture
		TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
		TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
		TIM_ICInitStructure.TIM_ICFilter = 0;

		TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;
		TIM_ICInit(TIM3, &TIM_ICInitStructure);

		TIM_ITConfig(TIM3, TIM_IT_CC2,ENABLE);

// ________________________________________________________________________________
// Startup

		TIM_DMAConfig(TIM1, TIM_DMABase_RCR, TIM_DMABurstLength_5Transfers);
		TIM_DMAConfig(TIM8, TIM_DMABase_RCR, TIM_DMABurstLength_5Transfers);

    TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Disable);
    TIM_OC2PreloadConfig(TIM1, TIM_OCPreload_Disable);
    TIM_OC3PreloadConfig(TIM1, TIM_OCPreload_Disable);
    TIM_OC4PreloadConfig(TIM1, TIM_OCPreload_Disable);
    TIM_OC1PreloadConfig(TIM8, TIM_OCPreload_Disable);
    TIM_OC2PreloadConfig(TIM8, TIM_OCPreload_Disable);
    TIM_OC3PreloadConfig(TIM8, TIM_OCPreload_Disable);
    TIM_OC4PreloadConfig(TIM8, TIM_OCPreload_Disable);

		TIM_CtrlPWMOutputs(TIM3, ENABLE);

		TIM_DMACmd(TIM1, TIM_DMA_Update, ENABLE);
		TIM_DMACmd(TIM8, TIM_DMA_Update, ENABLE);

// enable outputs, brez pulzov!!!
		TIM_SelectMasterSlaveMode(TIM1, TIM_MasterSlaveMode_Enable);	// T1 -> master mode
		TIM_SelectOutputTrigger(TIM1, TIM_TRGOSource_Enable); 				// triggers T8 from enable event

// trigger T8 za T1,DAC in ADC !!!
		TIM_SelectSlaveMode(TIM8, TIM_SlaveMode_Trigger); 						// T8 -> slave mode
		TIM_SelectInputTrigger(TIM8, TIM_TS_ITR0); 										// started from T1
		TIM_SelectOutputTrigger(TIM8, TIM_TRGOSource_Update);					// triggers ADC, DAC on update

		TIM_Cmd(TIM1,ENABLE);
		TIM_Cmd(TIM3,ENABLE);
		}
// ________________________________________________________________________________
// ________________________________________________________________________________
// ________________________________________________________________________________
// ________________________________________________________________________________
// ________________________________________________________________________________
// ________________________________________________________________________________
// ________________________________________________________________________________
// ________________________________________________________________________________
// ________________________________________________________________________________
// ________________________________________________________________________________
// ___ 100 khz interrupt __________________________________________________________
// ________________________________________________________________________________

int			Pref1=0,Pref2=0;
void		TIM1_UP_TIM10_IRQHandler(void) {

static
int			m=0,																											// timer repetition rate register index index, DMA table
				n=0,																											// adc index
				io=0,
				xi1=0,
				xi2=0;

int 		i,j,k;
short		z1,z2;
int			ki=30,kp=0;

				TIM_ClearITPendingBit(TIM1, TIM_IT_Update);								// briai ISR flage
				for(i=j=0;j<ADC3_AVG;++j)																	// sestej zadnjih N merjenih vrednosti vrsne napetosti
					i+=(unsigned short)(ADC3_buf[j].HV);										// 

				if(!n || !TIM18_buf[n].n) 																// ce je to zacetek (ali konec) sekvence, vzemi to za referencno vrednost - hitrejse kot 
					io=pfm->Burst.HVo;																			// ce vzames zahtevano vrednost iz osnovnega objekta !!!!
					
				k=_MAX_BURST/_MAX_ADC_RATE - DMA2_Stream3->NDTR/2;				// izracunaj offset za ADC niz

// vmesni izracun vrednosti za timerje 
				z1 = TIM18_buf[n].T1;
				z2 = TIM18_buf[n].T3;

				if(_MODE(pfm,_U_LOOP)) {
					z1 = (z1 * io + i/2)/i;
					z2 = (z2 * io + i/2)/i;
					if(pfm->Burst.Time>200 && _MODE(pfm,_P_LOOP)) {

						if(k > 5 && Pref1 && TIM18_buf[n].T1 > pfm->Burst.Pdelay*2) {
							int dx=(Pref1 - ADC1_buf[k-5].U * ADC1_buf[k-5].I)/4096;
							xi1 += dx*ki;
							z1 += xi1/4096 + dx*kp/4096;
						}

						if(k > 5 && Pref2 && TIM18_buf[n].T3 > pfm->Burst.Pdelay*2) {
							int dx=(Pref2 - ADC2_buf[k-5].U * ADC2_buf[k-5].I)/4096;
							xi2 += dx*ki;
							z2 += xi2/4096 + dx*kp/4096;
						}

						if(k > 200 + pfm->Burst.Delay) {
							if(!Pref1 && TIM18_buf[n].T1 > 2*pfm->Burst.Pdelay) {
								int jU=0,jI=0;
								for(i=5;i<13;++i)	{
									jU+=ADC1_buf[k-i].U;
									jI+=ADC1_buf[k-i].I;
								}
								Pref1=jU*jI/64;
								xi1=0;
							}

							if(!Pref2 && TIM18_buf[n].T3 > 2*pfm->Burst.Pdelay) {
								int jU=0,jI=0;
								for(i=5;i<13;++i)	{
									jU+=ADC2_buf[k-i].U;
									jI+=ADC2_buf[k-i].I;
								}
								Pref2=jU*jI/64;
								xi2=0;
							}
						}
					}
				}

// vpis v timerje

				TIM8->CCR1 = TIM1->CCR1 = __max(0,__min(_PWM_RATE_HI, z1));
				TIM8->CCR3 = TIM1->CCR3 = __max(0,__min(_PWM_RATE_HI, z2));
				
				if(_MODE(pfm,_XLAP_SINGLE)) {
					TIM8->CCR2 = TIM1->CCR2 = TIM1->CCR1;
					TIM8->CCR4 = TIM1->CCR4 = TIM1->CCR3;
				} else {
					TIM8->CCR2 = TIM1->CCR2 = _PWM_RATE_HI-TIM1->CCR1;
					TIM8->CCR4 = TIM1->CCR4 = _PWM_RATE_HI-TIM1->CCR3;
				}

				if(m++ == TIM18_buf[n].n/2) {
					m=0;
					if(TIM18_buf[n++].n == 0) {															// eof pulse train
						n=0;
						TIM_ITConfig(TIM1, TIM_IT_Update,DISABLE);
						_SET_EVENT(pfm,_PULSE_FINISHED);
						_CLEAR_MODE(pfm,_PULSE_INPROC);
					}
				}
				if(TIM1->CCR1==TIM1->ARR || TIM1->CCR3==TIM8->ARR)				// duty cycle 100% = PSRDYN error
 					_SET_ERROR(pfm,PFM_ERR_PSRDYN);
}
/*******************************************************************************/
/**
	* @brief	TIM3 IC2 ISR
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
void		TIM3_IRQHandler(void) {
				if(TIM_GetITStatus(TIM3,TIM_IT_CC2)==SET) {
					TIM_ClearITPendingBit(TIM3, TIM_IT_CC2);
					_SET_EVENT(pfm,_FAN_TACHO);
				}
}
/*******************************************************************************/
/**
  * @brief  This function handles External line 8 interrupt request (IGBT driver fault). Disables all PWM outputs
	* sets _IGBT_FAULT error event and locks the fault indicator
  * @param  None
  * @retval None
  */
void 		EXTI9_5_IRQHandler(void)
{
				EXTI_ClearITPendingBit(EXTI_Line8);
				_SET_ERROR(pfm,PFM_ERR_DRVERR);
}
/*******************************************************************************/
/**
  * @brief  This function handles External line 14 interrupt request (crowbar error)
	* Disables all PWM outputs & sets PFM_ERR_PULSEENABLE error event
  * @param  None
  * @retval None
  */
void 		EXTI15_10_IRQHandler(void)
{
				EXTI_ClearITPendingBit(EXTI_Line14);
#ifdef __DISCO__
				if(!_PFM_CWBAR_SENSE) {
#else
				if(_PFM_CWBAR_SENSE) {
#endif
					_SET_STATUS(pfm,_PFM_CWBAR_STAT);
					_CLEAR_ERROR(pfm, _CRITICAL_ERR_MASK);
					TIM_CtrlPWMOutputs(TIM1, ENABLE);
					TIM_CtrlPWMOutputs(TIM8, ENABLE);		
				}	else {
					_CLEAR_STATUS(pfm,_PFM_CWBAR_STAT);
					_SET_ERROR(pfm,PFM_ERR_PULSEENABLE);
				}
}
/*******************************************************************************/
/**
	* @brief	Trigger call
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
void		Trigger2(void) {
int			i;

				while(!(TIM1->CR1 & TIM_CR1_DIR));
				while((TIM1->CR1 & TIM_CR1_DIR));

				DMA_Cmd(DMA1_Stream5, DISABLE);
				DMA_Cmd(DMA2_Stream1, DISABLE);
				DMA_Cmd(DMA2_Stream5, DISABLE);

				while(DMA_GetCmdStatus(DMA1_Stream5) != DISABLE);
				while(DMA_GetCmdStatus(DMA2_Stream1) != DISABLE);
				while(DMA_GetCmdStatus(DMA2_Stream5) != DISABLE);

				for(i=0;TIM18_buf[i].n;++i);
				DMA_SetCurrDataCounter(DMA1_Stream5, i);										// 			dac
				DMA_SetCurrDataCounter(DMA2_Stream1, 5*i);									// 			tim8
				DMA_SetCurrDataCounter(DMA2_Stream5, 5*i);									//			tim1

				DMA_ClearFlag(DMA1_Stream5, DMA_FLAG_HTIF5 | DMA_FLAG_TEIF5 | DMA_FLAG_DMEIF5	| DMA_FLAG_FEIF5 | DMA_FLAG_TCIF5);
				DMA_ClearFlag(DMA2_Stream1, DMA_FLAG_HTIF1 | DMA_FLAG_TEIF1 | DMA_FLAG_DMEIF1	| DMA_FLAG_FEIF1 | DMA_FLAG_TCIF1);
				DMA_ClearFlag(DMA2_Stream5, DMA_FLAG_HTIF5 | DMA_FLAG_TEIF5 | DMA_FLAG_DMEIF5	| DMA_FLAG_FEIF5 | DMA_FLAG_TCIF5);

				ADC_ContinuousModeCmd(ADC1, ENABLE);
				ADC_ContinuousModeCmd(ADC2, ENABLE);
				ADC1->CR2 |= ADC_ExternalTrigConvEdge_Rising;
				ADC2->CR2 |= ADC_ExternalTrigConvEdge_Rising;

				DMA_Cmd(DMA2_Stream5, ENABLE);															//			tim1
				DMA_Cmd(DMA2_Stream1, ENABLE);															//			tim8
				DMA_Cmd(DMA1_Stream5, ENABLE);															//			dac
}

//______________________________________________________________________________
void		Trigger(PFM *p) {
				if(_MODE(p,_PULSE_INPROC))
					_DEBUG_MSG("trigger aborted...");
				else {
					ADC_DMARequestAfterLastTransferCmd(ADC1, DISABLE);				// at least ADC conv. time before ADC/DMA change 
					ADC_DMARequestAfterLastTransferCmd(ADC2, DISABLE);
					_SET_MODE(p,_PULSE_INPROC);
					_DEBUG_MSG("trigger...");
					SetSimmerRate(p, _PWM_RATE_HI);
				}
}
/**
* @}
*/ 
