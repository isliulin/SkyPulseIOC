/**
  ******************************************************************************
  * @file    adc.c
  * @author  Fotona d.d.
  * @version V1
  * @date    30-Sept-2013
  * @brief	 AD & DMA converters initialization
  *
  */
/** @addtogroup PFM6_Setup
* @{
*/
#include	"pfm.h"
_ADC3DMA	ADC3_buf[ADC3_AVG];
_ADCDMA		ADC1_buf[_MAX_BURST/_uS],
					ADC2_buf[_MAX_BURST/_uS],
					ADC1_simmer,
					ADC2_simmer;

#ifdef		__DISCO__
_ADC3DMA	ADC3_dummy[ADC3_AVG];
#endif

int		_ADCRates[]={3,15,28,56,84,112,144,480};

void	TriggerADC(PFM *p) {
	
			ADC_Cmd(ADC1, DISABLE);							ADC_Cmd(ADC2, DISABLE);
			DMA_Cmd(DMA2_Stream4,DISABLE);			DMA_Cmd(DMA2_Stream3,DISABLE);

			while(DMA_GetCmdStatus(DMA2_Stream4) != DISABLE);
			while(DMA_GetCmdStatus(DMA2_Stream3) != DISABLE);
	
			if(p) {
				DMA_MemoryTargetConfig(DMA2_Stream3,(uint32_t)ADC2_buf,DMA_Memory_0);
				DMA_MemoryTargetConfig(DMA2_Stream4,(uint32_t)ADC1_buf,DMA_Memory_0);
				DMA_SetCurrDataCounter(DMA2_Stream3,sizeof(ADC2_buf)/sizeof(short));
				DMA_SetCurrDataCounter(DMA2_Stream4,sizeof(ADC1_buf)/sizeof(short));

				DMA_ClearITPendingBit(DMA2_Stream3,DMA_IT_TCIF3|DMA_IT_HTIF3|DMA_IT_TEIF3|DMA_IT_DMEIF3|DMA_IT_FEIF3);
				DMA_ClearITPendingBit(DMA2_Stream4,DMA_IT_TCIF4|DMA_IT_HTIF4|DMA_IT_TEIF4|DMA_IT_DMEIF4|DMA_IT_FEIF4);
				DMA_ITConfig(DMA2_Stream4, DMA_IT_TC, ENABLE);
				ADC_AnalogWatchdogThresholdsConfig(ADC1,p->Burst.Imax,0);
				ADC_AnalogWatchdogThresholdsConfig(ADC2,p->Burst.Imax,0);	
			} else {
				DMA_MemoryTargetConfig(DMA2_Stream3,(uint32_t)&ADC2_simmer,DMA_Memory_0);
				DMA_MemoryTargetConfig(DMA2_Stream4,(uint32_t)&ADC1_simmer,DMA_Memory_0);
				DMA_SetCurrDataCounter(DMA2_Stream3,sizeof(_ADCDMA)/sizeof(short));
				DMA_SetCurrDataCounter(DMA2_Stream4,sizeof(_ADCDMA)/sizeof(short));

				DMA_ITConfig(DMA2_Stream4, DMA_IT_TC, DISABLE);
				ADC_AnalogWatchdogThresholdsConfig(ADC1,p->Burst.Isimm,0);
				ADC_AnalogWatchdogThresholdsConfig(ADC2,p->Burst.Isimm,0);	
				
				ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);
				ADC_DMARequestAfterLastTransferCmd(ADC2, ENABLE);	
			}
			DMA_ClearFlag(DMA2_Stream3,DMA_FLAG_TCIF3|DMA_FLAG_HTIF3|DMA_FLAG_TEIF3|DMA_FLAG_DMEIF3|DMA_FLAG_FEIF3);
			DMA_ClearFlag(DMA2_Stream4,DMA_FLAG_TCIF4|DMA_FLAG_HTIF4|DMA_FLAG_TEIF4|DMA_FLAG_DMEIF4|DMA_FLAG_FEIF4);

			DMA_Cmd(DMA2_Stream4,ENABLE);				DMA_Cmd(DMA2_Stream3,ENABLE);
			ADC_DMACmd(ADC1, DISABLE);					ADC_DMACmd(ADC2, DISABLE);
			ADC_DMACmd(ADC1, ENABLE);						ADC_DMACmd(ADC2, ENABLE);
			ADC_Cmd		(ADC1, ENABLE);						ADC_Cmd		(ADC2, ENABLE);
}
/*******************************************************************************/
void	DMA2_Stream4_IRQHandler(void) {
			TriggerADC(NULL);
			_SET_EVENT(pfm,_ADC_FINISHED);
}
/*******************************************************************************/
/**
  * @brief  ADC1 channel init. with DMA
  * @param  None
  * @retval None
  */
void 	Initialize_ADC1(void)
{
			ADC_InitTypeDef       ADC_InitStructure;
			DMA_InitTypeDef       DMA_InitStructure;
			GPIO_InitTypeDef      GPIO_InitStructure;

/* DMA2 Stream4 channel0 configuration **************************************/
			RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);

			DMA_InitStructure.DMA_Channel = DMA_Channel_0;
			DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(ADC1_BASE+0x4C);
			DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&ADC1_simmer;
			DMA_InitStructure.DMA_BufferSize = sizeof(_ADCDMA)/sizeof(short);
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
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
			GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
			GPIO_Init(GPIOB, &GPIO_InitStructure);

/* ADC1 Init ****************************************************************/
			ADC_StructInit(&ADC_InitStructure);
			ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
			ADC_InitStructure.ADC_ScanConvMode = ENABLE;
			ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
			ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Rising;
			ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T8_TRGO;

			ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
			ADC_InitStructure.ADC_NbrOfConversion = sizeof(_ADCDMA)/sizeof(short);
			ADC_Init(ADC1, &ADC_InitStructure);

/* ADC1 regular channel12 configuration *************************************/
			ADC_RegularChannelConfig(ADC1, ADC_Channel_9, 1, ADC_SampleTime_3Cycles);				// Uflash 1
			ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 2, ADC_SampleTime_3Cycles);				// Iflash 1

			ADC_AnalogWatchdogSingleChannelConfig(ADC1,ADC_Channel_8);
			ADC_AnalogWatchdogCmd(ADC1,ADC_AnalogWatchdog_SingleRegEnable);

/* Enable DMA request after last transfer (Single-ADC mode) */
			ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);
			ADC_Cmd(ADC1, ENABLE);
			ADC_DMACmd(ADC1, ENABLE);
}
/*******************************************************************************/
/**
  * @brief  ADC2 channel init. with DMA
  * @param  None
  * @retval None
  */
void 	Initialize_ADC2(void)
{
			ADC_InitTypeDef       ADC_InitStructure;
			DMA_InitTypeDef       DMA_InitStructure;
			GPIO_InitTypeDef      GPIO_InitStructure;

/* DMA2 Stream3 channel1 configuration **************************************/
			RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);

			DMA_InitStructure.DMA_Channel = DMA_Channel_1;
			DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(ADC2_BASE+0x4C);
			DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&ADC2_simmer;
			DMA_InitStructure.DMA_BufferSize = sizeof(_ADCDMA)/sizeof(short);
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
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
			GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
			GPIO_Init(GPIOC, &GPIO_InitStructure);

/* ADC2 Init ****************************************************************/
			ADC_StructInit(&ADC_InitStructure);
			ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
			ADC_InitStructure.ADC_ScanConvMode = ENABLE;
			ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
			ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Rising;
			ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T8_TRGO;

			ADC_InitStructure.ADC_NbrOfConversion = sizeof(_ADCDMA)/sizeof(short);
			ADC_Init(ADC2, &ADC_InitStructure);

/* ADC2 regular channel12 configuration *************************************/
			ADC_RegularChannelConfig(ADC2, ADC_Channel_15, 1, ADC_SampleTime_3Cycles);		// Uflash 2
			ADC_RegularChannelConfig(ADC2, ADC_Channel_14, 2, ADC_SampleTime_3Cycles);		// Iflash 2

			ADC_AnalogWatchdogSingleChannelConfig(ADC2,ADC_Channel_14);	
			ADC_AnalogWatchdogCmd(ADC2,ADC_AnalogWatchdog_SingleRegEnable);

/* Enable DMA request after last transfer (Single-ADC mode) */
			ADC_DMARequestAfterLastTransferCmd(ADC2, ENABLE);
			ADC_Cmd(ADC2, ENABLE);
			ADC_DMACmd(ADC2, ENABLE);
}
/*******************************************************************************/
/**
  * @brief  ADC3 channel init. with DMA
  * @param  None
  * @retval None
  */
void 	Initialize_ADC3(void)
{
			ADC_InitTypeDef       ADC_InitStructure;
			DMA_InitTypeDef       DMA_InitStructure;
			GPIO_InitTypeDef      GPIO_InitStructure;

/* Enable ADC3, DMA2 and GPIO clocks ****************************************/
			RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2 | RCC_AHB1Periph_GPIOC, ENABLE);
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3, ENABLE);

/* DMA2 Stream0 channel0 configuration **************************************/
			DMA_InitStructure.DMA_Channel = DMA_Channel_2;
			DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(ADC3_BASE+0x4C);
#ifdef		__DISCO__
			DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)ADC3_dummy;
#else			
			DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)ADC3_buf;
#endif
			DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
			DMA_InitStructure.DMA_BufferSize = sizeof(ADC3_buf)/sizeof(short);
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
			DMA_Init(DMA2_Stream0, &DMA_InitStructure);
			DMA_Cmd(DMA2_Stream0, ENABLE);

/* Configure ADC3 Channel12 PC3,PA0,PA1 pin as analog input ******************************/

			GPIO_StructInit(&GPIO_InitStructure);
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
			GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;

			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2;											// temp 1, temp 2, HV/2														
			GPIO_Init(GPIOA, &GPIO_InitStructure);
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;											// HV,20V,-5V
			GPIO_Init(GPIOC, &GPIO_InitStructure);

/* ADC3 Init ****************************************************************/
			ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
			ADC_InitStructure.ADC_ScanConvMode = ENABLE;
			ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
			ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
			ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
			ADC_InitStructure.ADC_NbrOfConversion = sizeof(_ADC3DMA)/sizeof(short);
			ADC_Init(ADC3, &ADC_InitStructure);

/* ADC3 regular channel12 configuration *************************************/
			ADC_RegularChannelConfig(ADC3, ADC_Channel_0,  1, ADC_SampleTime_3Cycles);							// temp 1
			ADC_RegularChannelConfig(ADC3, ADC_Channel_1,  2, ADC_SampleTime_3Cycles);							// temp 2
			ADC_RegularChannelConfig(ADC3, ADC_Channel_2,  3, ADC_SampleTime_3Cycles);							// HV/2
			ADC_RegularChannelConfig(ADC3, ADC_Channel_11, 4, ADC_SampleTime_3Cycles);							// HV
			ADC_RegularChannelConfig(ADC3, ADC_Channel_12, 5, ADC_SampleTime_3Cycles);							// +20V
			ADC_RegularChannelConfig(ADC3, ADC_Channel_13, 6, ADC_SampleTime_3Cycles);							// -5V
#ifndef __DISCO__
			ADC_AnalogWatchdogSingleChannelConfig(ADC3,ADC_Channel_11);
			ADC_AnalogWatchdogCmd(ADC3,ADC_AnalogWatchdog_SingleRegEnable);	
#endif
/* Enable DMA request after last transfer (Single-ADC mode) */
			ADC_DMARequestAfterLastTransferCmd(ADC3, ENABLE);
			ADC_DMACmd(ADC3, ENABLE);
			ADC_Cmd(ADC3, ENABLE);
			ADC_SoftwareStartConv(ADC3);
}
/**
  * @brief  ADC	common init
  * @param  None
  * @retval None
  */
/*******************************************************************************/
void 	Initialize_ADC(void)
{
			ADC_CommonInitTypeDef ADC_CommonInitStructure;
			RCC_APB2PeriphClockCmd(	RCC_APB2Periph_ADC |
									RCC_APB2Periph_ADC1 |
									RCC_APB2Periph_ADC2 |
									RCC_APB2Periph_ADC3, ENABLE);

			ADC_CommonStructInit(&ADC_CommonInitStructure);
			ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
			ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
			ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
			ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
			ADC_CommonInit(&ADC_CommonInitStructure);

			Initialize_ADC1();
			Initialize_ADC2();
			Initialize_ADC3();
}
/*******************************************************************************
* Function Name  : ADC_IRQHandler
* Description    : ADC3 analog watchdog ISR
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void ADC_IRQHandler(void)	{
		if(ADC_GetITStatus(ADC1, ADC_IT_AWD) != RESET) {
			ADC_ClearITPendingBit(ADC1, ADC_IT_AWD);
			_SET_ERROR(pfm,_PFM_ADCWDG_ERR);
		}
		
		if(ADC_GetITStatus(ADC2, ADC_IT_AWD) != RESET) {
			ADC_ClearITPendingBit(ADC2, ADC_IT_AWD);
			_SET_ERROR(pfm,_PFM_ADCWDG_ERR);
		}
		if(ADC_GetITStatus(ADC3, ADC_IT_AWD) != RESET) {
			ADC_ClearITPendingBit(ADC3, ADC_IT_AWD);
				_SET_ERROR(pfm,_PFM_ADCWDG_ERR);
				_SET_EVENT(pfm,_ADC_WATCHDOG);

		}
}

/**
* @}
*/ 
