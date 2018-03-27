#include	"adc.h"
#include	"gpio.h"
#include	"ioc.h"
#include	<stdlib.h>
/**
  ******************************************************************************
  * @file    adc.cpp
  * @version 
  * @date    
  * @brief	 AD & DMA converters initialization
  *
  */
/** @addtogroup
* @{
*/
_ADC	 *_ADC::instance=NULL;		
_ADMA		_ADC::adc[16],_ADC::adf,_ADC::offset,_ADC::gain;
_DIODE	_ADC::diode[160];
/**
  * @brief  ADC	common init, can be executed only once
  * @param  None
  * @retval None
  */
/*******************************************************************************/
void 	_ADC::Initialize_ADC2(void) {
#if defined(__IOC_V2__)
			ADC_InitTypeDef       ADC_InitStructure;
			DMA_InitTypeDef       DMA_InitStructure;
			GPIO_InitTypeDef      GPIO_InitStructure;

			DMA_DeInit(DMA2_Stream3);
	
			DMA_InitStructure.DMA_Channel = DMA_Channel_1;
			DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(ADC2_BASE+0x4C);
			DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)diode;
			DMA_InitStructure.DMA_BufferSize = sizeof(diode)/sizeof(short);
			DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
			DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
			DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
			DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
			DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
			DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
			DMA_InitStructure.DMA_Priority = DMA_Priority_High;
			DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;
			DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
			DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
			DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
			DMA_Init(DMA2_Stream3, &DMA_InitStructure);
			DMA_Cmd(DMA2_Stream3, ENABLE);

/* Configure ADC2 Channel gpio pin as analog inputs *************************/

			GPIO_StructInit(&GPIO_InitStructure);
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
			GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
			GPIO_Init(GPIOC, &GPIO_InitStructure);

/* ADC2 Init ****************************************************************/
			ADC_StructInit(&ADC_InitStructure);
			ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
			ADC_InitStructure.ADC_ScanConvMode = ENABLE;
			ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
			ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;

			ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
			ADC_InitStructure.ADC_NbrOfConversion = sizeof(_DIODE)/sizeof(short);
			ADC_Init(ADC2, &ADC_InitStructure);

/* ADC2 regular channel12 configuration *************************************/
			ADC_RegularChannelConfig(ADC2, ADC_Channel_14,  1, ADC_SampleTime_56Cycles);
			ADC_RegularChannelConfig(ADC2, ADC_Channel_15,  2, ADC_SampleTime_56Cycles);
			
/* Enable DMA request after last transfer (Single-ADC mode) */
			ADC_DMARequestAfterLastTransferCmd(ADC2, ENABLE);
			ADC_Cmd(ADC2, ENABLE);
			ADC_DMACmd(ADC2, ENABLE);
			ADC_SoftwareStartConv(ADC2);
			
// IT handling	
			NVIC_EnableIRQ(DMA2_Stream3_IRQn);
			DMA_ITConfig(DMA2_Stream3, DMA_IT_TC | DMA_IT_HT, ENABLE);
#endif
}
/**
  * @brief  ADC	common init, can be executed only once
  * @param  None
  * @retval None
  */
/*******************************************************************************/
void	_ADC::Initialize_ADC1() {
			ADC_InitTypeDef       ADC_InitStructure;
			DMA_InitTypeDef       DMA_InitStructure;
			GPIO_InitTypeDef      GPIO_InitStructure;

			DMA_DeInit(DMA2_Stream4);
	
			DMA_InitStructure.DMA_Channel = DMA_Channel_0;
			DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(ADC1_BASE+0x4C);
			DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)adc;
			DMA_InitStructure.DMA_BufferSize = sizeof(adc)/sizeof(short);
			DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
			DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
			DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
			DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
			DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
			DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
			DMA_InitStructure.DMA_Priority = DMA_Priority_High;
			DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;
			DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
			DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
			DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;

			DMA_Init(DMA2_Stream4, &DMA_InitStructure);

			DMA_Cmd(DMA2_Stream4, ENABLE);

/* Configure ADC1 Channel gpio pin as analog inputs *************************/

			GPIO_StructInit(&GPIO_InitStructure);
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
			GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;															// T2,T3,V5;
			GPIO_Init(GPIOA, &GPIO_InitStructure);
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;	 																				// V12,V24;
			GPIO_Init(GPIOB, &GPIO_InitStructure);

#if defined(__IOC_V2__)
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;																												// pump current sense;
			GPIO_Init(GPIOA, &GPIO_InitStructure);
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;								// P0,P1,P2,P3;
			GPIO_Init(GPIOC, &GPIO_InitStructure);
#else
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4;		// P0,P1,P2,P3, pump current sense;
			GPIO_Init(GPIOC, &GPIO_InitStructure);
#endif

/* ADC1 Init ****************************************************************/
			ADC_StructInit(&ADC_InitStructure);
			ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
			ADC_InitStructure.ADC_ScanConvMode = ENABLE;
			ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
			ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;

			ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
			ADC_InitStructure.ADC_NbrOfConversion = sizeof(_ADMA)/sizeof(short);
			ADC_Init(ADC1, &ADC_InitStructure);

/* ADC1 regular channel12 configuration *************************************/
			ADC_RegularChannelConfig(ADC1, ADC_Channel_1,  1, ADC_SampleTime_84Cycles);
			ADC_RegularChannelConfig(ADC1, ADC_Channel_2,  2, ADC_SampleTime_84Cycles);
			ADC_RegularChannelConfig(ADC1, ADC_Channel_3,  3, ADC_SampleTime_84Cycles);
			ADC_RegularChannelConfig(ADC1, ADC_Channel_8,  4, ADC_SampleTime_84Cycles);
			ADC_RegularChannelConfig(ADC1, ADC_Channel_9,  5, ADC_SampleTime_84Cycles);
			ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 6, ADC_SampleTime_84Cycles);
			ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 7, ADC_SampleTime_84Cycles);
			ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 8, ADC_SampleTime_84Cycles);
			ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 9, ADC_SampleTime_84Cycles);
#if defined(__IOC_V2__)
			ADC_RegularChannelConfig(ADC1, ADC_Channel_0,10, 	ADC_SampleTime_84Cycles);
#else
			ADC_RegularChannelConfig(ADC1, ADC_Channel_14,10, ADC_SampleTime_84Cycles);
#endif
/* Enable DMA request after last transfer (Single-ADC mode) */
			ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);
			ADC_Cmd(ADC1, ENABLE);
			ADC_DMACmd(ADC1, ENABLE);
			ADC_SoftwareStartConv(ADC1);
// IT handling	
			NVIC_EnableIRQ(DMA2_Stream4_IRQn);
			DMA_ITConfig(DMA2_Stream4, DMA_IT_TC, ENABLE);
}
/*******************************************************************************
* Function Name	:
* Description		: 
* Output				:
* Return				: None
*******************************************************************************/
int		_ADC::Th2o() {
			return __fit(adf.T2,Rtab,Ttab);
}
/*******************************************************************************
* Function Name	:
* Description		: 
* Output				:
* Return				: None
*******************************************************************************/
int		_ADC::Status() {
int		e=_NOERR;

#if defined (__IOC_V2__)
			if(_SYS_SHG_ENABLED && GPIO_ReadInputDataBit(_cwbPort,_cwbButton) == Bit_RESET) {
#else
			if(_SYS_SHG_ENABLED && GPIO_ReadInputDataBit(_cwbPort,_cwbButton) == Bit_SET) {
#endif
#if defined (_cwbHandpc) && defined (_cwbDoor)
				if(GPIO_ReadInputDataBit(_cwbPort,_cwbHandpc) == Bit_RESET)
					e |= _handpcDisabled;
				else if(GPIO_ReadInputDataBit(_cwbPort,_cwbDoor) == Bit_RESET)
					e |= _doorswDisabled;
				else
#endif
				e |= _emgDisabled;
			}
	
			if(__time__ > _ADC_ERR_DELAY) {
				if(abs(adf.V5  - _V5to16X)	> _V5to16X/10)
					e |= _V5;
				if(abs(adf.V12 - _V12to16X) > _V12to16X/5)
					e |= _V12;
				if(abs(adf.V24 - _V24to16X) > _V24to16X/10)
					e |= _V24;
				if(Th2o() > 50*100)
					e |= _sysOverheat;
			}	
			return e;
}
/**
  * @brief  ADC	common init, can be executed only once
  * @param  None
  * @retval None
  */
/*******************************************************************************/
_ADC::_ADC() {
			ADC_CommonInitTypeDef ADC_CommonInitStructure;
			if(instance==NULL) {
				instance=this;
				ADC_DeInit();
				RCC_APB2PeriphClockCmd(	RCC_APB2Periph_ADC | RCC_APB2Periph_ADC1 | RCC_APB2Periph_ADC2 ,ENABLE);
				RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1 | RCC_AHB1Periph_DMA2, ENABLE);

				ADC_CommonStructInit(&ADC_CommonInitStructure);
				ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
				ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4;
				ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
				ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
				ADC_CommonInit(&ADC_CommonInitStructure);

				n=timeout=0;
				Initialize_ADC1();
				Initialize_ADC2();
			}
}
/*******************************************************************************
* Function Name	:
* Description		:	TIM3 pump && fan tacho IC
* Output				:
* Return				:
*******************************************************************************/
void	_ADC::adcFilter() {
			memset(&adf,0,sizeof(adf));
			for(int i=0; i<sizeof(adc)/sizeof(_ADMA); ++i) {
				_ADC::adf.T2					+= _ADC::adc[i].T2;
				_ADC::adf.T3					+= _ADC::adc[i].T3;
				_ADC::adf.V5					+= _ADC::adc[i].V5;
				_ADC::adf.V12					+= _ADC::adc[i].V12;
				_ADC::adf.V24					+= _ADC::adc[i].V24;
				_ADC::adf.cooler			+= _ADC::adc[i].cooler;
				_ADC::adf.bottle			+= _ADC::adc[i].bottle;
				_ADC::adf.compressor	+= _ADC::adc[i].compressor;
				_ADC::adf.air					+= _ADC::adc[i].air;
				_ADC::adf.Ipump				+= _ADC::adc[i].Ipump;
			}
}
/*******************************************************************************
* Function Name	:
* Description		:	TIM3 pump && fan tacho IC
* Output				:
* Return				:
*******************************************************************************/
float x,dx,k=0.0006250;
float fo=15e6/(12+56)/2;

// double t = Ton / (1 - exp(-fo * k * Ton));
// 9.06666666666666666666666 us
								
void		_ADC::diodeFilter(int half) {
_DIODE	*d=diode;
int			n=sizeof(diode)/sizeof(_DIODE)/2;
				if(half)
					d=&diode[n];
				
//				DAC_SetChannel1Data(DAC_Align_12b_R,x);
			
				while(n--) {
					dx += k*(d++->D1 - x-dx*(float)2);
					x += k*dx;	
				}
//				DAC_SetChannel1Data(DAC_Align_12b_L,0);
}
/*******************************************************************************
* Function Name	:
* Description		:	TIM3 pump && fan tacho IC
* Output				:
* Return				:
*******************************************************************************/
extern	"C" {
void	DMA2_Stream3_IRQHandler(void) {
			if(DMA_GetITStatus(DMA2_Stream3,DMA_IT_TCIF3)==SET) {
				_ADC::diodeFilter(1);
			}
			if(DMA_GetITStatus(DMA2_Stream3,DMA_IT_HTIF3)==SET) {
				_ADC::diodeFilter(0);
			}
			DMA_ClearITPendingBit( DMA2_Stream3,DMA_IT_TCIF3|DMA_IT_HTIF3|DMA_IT_TEIF3|DMA_IT_DMEIF3|DMA_IT_FEIF3 );
}
/*******************************************************************************
* Function Name	:
* Description		:	TIM3 pump && fan tacho IC
* Output				:
* Return				:
*******************************************************************************/
void	DMA2_Stream4_IRQHandler(void) {
			if(DMA_GetITStatus(DMA2_Stream4,DMA_IT_TCIF4)==SET)
				_ADC::adcFilter();
			DMA_ClearITPendingBit( DMA2_Stream4,DMA_IT_TCIF4 | DMA_IT_HTIF4 | DMA_IT_TEIF4 | DMA_IT_DMEIF4 | DMA_IT_FEIF4 );
}
}
/**
* @}
*/ 

