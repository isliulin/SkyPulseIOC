#include	"adc.h"
#include	"isr.h"
#include	<stdlib.h>
/**
  ******************************************************************************
  * @file    adc.cpp
  * @author  Fotona d.d.
  * @version 
  * @date    
  * @brief	 AD & DMA converters initialization
  *
  */
/** @addtogroup
* @{
*/

_ADC 		*_ADC::instance=NULL;		
_ADMA		 _ADC::buffer,_ADC::adf,_ADC::offset,_ADC::gain;

/**
  * @brief  ADC	common init, can be executed only once
  * @param  None
  * @retval None
  */
/*******************************************************************************/
_ADC::_ADC() {
			if(instance==NULL) {
				instance=this;
#ifndef __SIMULATION__
				ADC_InitTypeDef       ADC_InitStructure;
				DMA_InitTypeDef       DMA_InitStructure;
				GPIO_InitTypeDef      GPIO_InitStructure;

				ADC_DeInit();
				ADC_CommonInitTypeDef ADC_CommonInitStructure;
				RCC_APB2PeriphClockCmd(	RCC_APB2Periph_ADC | RCC_APB2Periph_ADC1 ,ENABLE);

				ADC_CommonStructInit(&ADC_CommonInitStructure);
				ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
				ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
				ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
				ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
				ADC_CommonInit(&ADC_CommonInitStructure);

				RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1 | RCC_AHB1Periph_DMA2, ENABLE);

				DMA_DeInit(DMA2_Stream4);
	
				DMA_InitStructure.DMA_Channel = DMA_Channel_0;
				DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(ADC1_BASE+0x4C);
				DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&buffer;
				DMA_InitStructure.DMA_BufferSize = sizeof(buffer)/sizeof(short);
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
//			DMA_ITConfig(DMA2_Stream4, DMA_IT_TC | DMA_IT_HT, ENABLE);
//			DMA_ITConfig(DMA2_Stream4, DMA_IT_TC, ENABLE);

/* Configure ADC1 Channel gpio pin as analog inputs *************************/

				GPIO_StructInit(&GPIO_InitStructure);
				GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
				GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
				GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;															// T2,T3,V5;
				GPIO_Init(GPIOA, &GPIO_InitStructure);
				GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;	 																				// V12,V24;
				GPIO_Init(GPIOB, &GPIO_InitStructure);
				GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4;		// P0,P1,P2,P3, pump current sense;
				GPIO_Init(GPIOC, &GPIO_InitStructure);

/* ADC1 Init ****************************************************************/
				ADC_StructInit(&ADC_InitStructure);
				ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
				ADC_InitStructure.ADC_ScanConvMode = ENABLE;
				ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
				ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;

				ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Left;
				ADC_InitStructure.ADC_NbrOfConversion = sizeof(_ADMA)/sizeof(short);
				ADC_Init(ADC1, &ADC_InitStructure);
/* ADC1 regular channel12 configuration *************************************/
				ADC_RegularChannelConfig(ADC1, ADC_Channel_1,  1, ADC_SampleTime_112Cycles);
				ADC_RegularChannelConfig(ADC1, ADC_Channel_2,  2, ADC_SampleTime_112Cycles);
				ADC_RegularChannelConfig(ADC1, ADC_Channel_3,  3, ADC_SampleTime_112Cycles);
				ADC_RegularChannelConfig(ADC1, ADC_Channel_8,  4, ADC_SampleTime_112Cycles);
				ADC_RegularChannelConfig(ADC1, ADC_Channel_9,  5, ADC_SampleTime_112Cycles);
				ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 6, ADC_SampleTime_112Cycles);
				ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 7, ADC_SampleTime_112Cycles);
				ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 8, ADC_SampleTime_112Cycles);
				ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 9, ADC_SampleTime_112Cycles);
				ADC_RegularChannelConfig(ADC1, ADC_Channel_14,10, ADC_SampleTime_112Cycles);

/* Enable DMA request after last transfer (Single-ADC mode) */
				ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);
				ADC_Cmd(ADC1, ENABLE);
				ADC_DMACmd(ADC1, ENABLE);
				ADC_SoftwareStartConv(ADC1);
#endif
				n=timeout=0;
			}
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
int			_ADC::Status() {
int			e=0;

				adf.T2					+= (buffer.T2					- adf.T2)/16;
				adf.T3					+= (buffer.T3					- adf.T3)/16;
				adf.V5					+= (buffer.V5					- adf.V5)/16;
				adf.V12					+= (buffer.V12				- adf.V12)/16;
				adf.V24					+= (buffer.V24				- adf.V24)/16;
				adf.cooler			+= (buffer.cooler			- adf.cooler)/16;
				adf.bottle			+= (buffer.bottle			- adf.bottle)/16;
				adf.compressor	+= (buffer.compressor	- adf.compressor)/16;
				adf.air					+= (buffer.air				- adf.air)/16;
				adf.Ipump				+= (buffer.Ipump			- adf.Ipump)/16;

				if(abs(adf.V5  - _V5to16X)	> _V5to16X/10)
					_SET_BIT(e,V5);
				if(abs(adf.V12 - _V12to16X) > _V12to16X/5)
					_SET_BIT(e,V12);
				if(abs(adf.V24 - _V24to16X) > _V24to16X/10)
					_SET_BIT(e,V24);

				if(Th2o() > 50*100)
					_SET_BIT(e,sysOverheat);
				if(_EMG_DISABLED && _SYS_SHG_ENABLED)
					_SET_BIT(e,emgDisabled);
				return e;
}
/**
* @}
*/ 
