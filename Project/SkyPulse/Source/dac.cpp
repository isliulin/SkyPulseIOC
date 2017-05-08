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
#include	"dac.h"
#include	"math.h"
_DAC *_DAC::instance=NULL;
/*******************************************************************************/
/**
	* @brief	TIM3 IC2 ISR
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/

_DAC::_DAC() {
	if(instance == NULL) {
		instance=this;
#ifndef __SIMULATION__
		DAC_InitTypeDef		DAC_InitStructure;
		GPIO_InitTypeDef	GPIO_InitStructure;

		GPIO_StructInit(&GPIO_InitStructure);
		DAC_StructInit(&DAC_InitStructure);

		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);
		DAC_DeInit();
		DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
		DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
		DAC_Init(DAC_Channel_1, &DAC_InitStructure);
		DAC_Cmd(DAC_Channel_1, ENABLE);
#if defined (__DISCO__) || defined (__IOC_V1__)
		DAC_SetChannel1Data(DAC_Align_12b_R,0xfff);	
		DAC_SetChannel2Data(DAC_Align_12b_R,0xfff);	
		DAC_Init(DAC_Channel_2, &DAC_InitStructure);
		DAC_DualSoftwareTriggerCmd(ENABLE);
		DAC_Cmd(DAC_Channel_2, ENABLE);
#elif defined (__IOC_V2__)
		DAC_SetChannel1Data(DAC_Align_12b_R,0);	
		DAC_SetChannel2Data(DAC_Align_12b_R,0x7ff);	
		for(int i=0; i<sizeof(DacBuff)/sizeof(short); ++i)
			DacBuff[i]=0x7ff;	
		TIM6_Config();
		DAC_Ch2_Config();
#else
		***error: HW platform not defined
#endif
#endif
	}
}

/**
* @}
*/ 
/*--------------------------------------------------------------------*/
#define DAC_DHR12L2_ADDRESS    0x40007418
unsigned short DacBuff[100];
/*--------------------------------------------------------------------*/
/**
  * @brief  DAC  Channel2 SineWave Configuration
  * @param  None
  * @retval None
  */
void TIM6_Config(void)
{
  TIM_TimeBaseInitTypeDef    TIM_TimeBaseStructure;
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
  TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
  TIM_TimeBaseStructure.TIM_Period = SystemCoreClock/2/41000;
  TIM_TimeBaseStructure.TIM_Prescaler = 0;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; 
  TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);
  TIM_SelectOutputTrigger(TIM6, TIM_TRGOSource_Update);
  TIM_Cmd(TIM6, ENABLE);
}
/*--------------------------------------------------------------------*/
/**
  * @brief  DAC  Channel2 SineWave Configuration
  * @param  None
  * @retval None
  */
void DAC_Ch2_Config(void)
{
  DMA_InitTypeDef DMA_InitStructure;
  DAC_InitTypeDef DAC_InitStructure;

  DAC_InitStructure.DAC_Trigger = DAC_Trigger_T6_TRGO;
  DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
  DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
  DAC_Init(DAC_Channel_2, &DAC_InitStructure);
 
//  DMA_DeInit(DMA1_Stream6);
  DMA_InitStructure.DMA_Channel = DMA_Channel_7;  
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)DAC_DHR12L2_ADDRESS;
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)DacBuff;
  DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
  DMA_InitStructure.DMA_BufferSize = sizeof(DacBuff)/sizeof(short);
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(DMA1_Stream6, &DMA_InitStructure);

	DAC_SoftwareTriggerCmd(DAC_Channel_2, ENABLE);
	DMA_Cmd(DMA1_Stream6, ENABLE);
  DAC_Cmd(DAC_Channel_2, ENABLE);
	DAC_DMACmd(DAC_Channel_2, ENABLE);
		DAC_SoftwareTriggerCmd(DAC_Channel_2, ENABLE);
}
