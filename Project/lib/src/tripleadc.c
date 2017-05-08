
/* Private Include*/

#include	"pfm.h"
/* Private Defines*/

/* Privat Variable */
__IO uint16_t ADCConvertedValue[ADCSENSORn];

__IO uint32_t ADC3ConvertedVoltage=0;
__IO uint16_t ADC3ConvertedValue = 0;
__IO uint16_t ADC3ConvertedValue1 = 0;
__IO uint16_t ADC3ConvertedValue2= 0;


/*ADC Struct, contains the ADC Value Array and the Buffersize*/
struct ADCData dataSet1;
struct ADCData dataSet2;
struct ADCData dataSet3;

/* Function Area*/

//==============================================================================
// Konfiguration
//==============================================================================

// Set sample Time via recieve Command
// Set ADC Channel numbers via recieve Command
void TIM2_Configuration(void)
{ 
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
  
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
 
  /* Time base configuration */
  TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);               //500000
  TIM_TimeBaseStructure.TIM_Period = (84000000 / 200000) - 1; // 200 KHz, from 84 MHz TIM2CLK (ie APB1 = HCLK/4, TIM2CLK = HCLK/2)
  TIM_TimeBaseStructure.TIM_Prescaler = 0;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
 
  /* TIM2 TRGO selection */
  TIM_SelectOutputTrigger(TIM2, TIM_TRGOSource_Update); // ADC_ExternalTrigConv_T2_TRGO
 
  /* TIM2 enable counter */
  TIM_Cmd(TIM2, ENABLE);
}

void DMANvicInterruptConfig(void){
    // Interrupt Config 
  NVIC_InitTypeDef NVIC_InitStructure;
 
  /* Enable the DMA Stream IRQ Channel */
  NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream0_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  }

// INIT ADC
void InitADC(void){
  ADC_InitTypeDef       ADC_InitStructure;
  ADC_CommonInitTypeDef ADC_CommonInitStructure;
  DMA_InitTypeDef       DMA_InitStructure;
  GPIO_InitTypeDef      GPIO_InitStructure;
  
  // Enable GPIO Clocks, ADC1, ADC2, ADC3, DMA2
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2 | RCC_AHB1Periph_GPIOA | 
                         RCC_AHB1Periph_GPIOC, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_ADC2 | 
                         RCC_APB2Periph_ADC3, ENABLE);
  
 
  DMANvicInterruptConfig();
  TIM2_Configuration();
  // See DMA2 request mapping [STM32F407 reference Manual table 43
  // DMA2 Stream0 channel0 configuration DMAStream ADC1  
  DMA_InitStructure.DMA_Channel = DMA_Channel_0;  
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)ADC_CDR_ADDRESS;
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&ADCConvertedValue;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
  DMA_InitStructure.DMA_BufferSize = 6;
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
  
  
  //Enable DMA Stream Half / Transfer Complete interrupt 
  DMA_ITConfig(DMA2_Stream0, DMA_IT_TC | DMA_IT_HT, ENABLE);
  DMA_Cmd(DMA2_Stream0, ENABLE);
  // GPIO Init ADC1 --> PA3; PA4; PA5; PA6
  
  GPIO_InitStructure.GPIO_Pin = ADCSENSOR1_PIN|ADCSENSOR2_PIN | ADCSENSOR3_PIN|ADCSENSOR4_PIN;;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;  // Set PA3 and PA4 as Analog Input
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  // GPIO Init ADC3 --> PC0; PC2;
  GPIO_InitStructure.GPIO_Pin = ADCSENSOR5_PIN|ADCSENSOR6_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  
  // Common ADC Init
  ADC_CommonInitStructure.ADC_Mode = ADC_TripleMode_RegSimult;//;
  ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
  ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_1; //DMA mode 1 enabled (2 / 3 half-words one by one - 1 then 2 then 3)
  ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
  ADC_CommonInit(&ADC_CommonInitStructure);
  
  // Config the Master ADC
  ADC_InitStructure.ADC_Resolution = ADC_Resolution_8b;
  ADC_InitStructure.ADC_ScanConvMode = DISABLE;//DISABLE
  ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
  ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Rising;
  ADC_InitStructure.ADC_ExternalTrigConv =ADC_ExternalTrigConv_T2_TRGO; // Trigger Event
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfConversion = 2;//2
  // Init ADC1  
  ADC_Init(ADC1, &ADC_InitStructure);
  // Config the slave ADC
  ADC_InitStructure.ADC_Resolution = ADC_Resolution_8b;
  ADC_InitStructure.ADC_ScanConvMode = DISABLE;//DISABLE
  ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
  ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfConversion = 2;//2
  // Init ADC2  
  ADC_Init(ADC2, &ADC_InitStructure);
  // Init ADC3  
  ADC_Init(ADC3, &ADC_InitStructure);

  
  // ADC regular channel configuration 
  ADC_RegularChannelConfig(ADCSENSOR1_ADC, ADCSENSOR1_CHANNEL, 1, ADDCSENSORSampeTime);
  ADC_RegularChannelConfig(ADCSENSOR2_ADC, ADCSENSOR2_CHANNEL, 2, ADDCSENSORSampeTime);
  ADC_RegularChannelConfig(ADCSENSOR3_ADC, ADCSENSOR3_CHANNEL, 1, ADDCSENSORSampeTime);
  ADC_RegularChannelConfig(ADCSENSOR4_ADC, ADCSENSOR4_CHANNEL, 2, ADDCSENSORSampeTime);
  ADC_RegularChannelConfig(ADCSENSOR5_ADC, ADCSENSOR5_CHANNEL, 1, ADDCSENSORSampeTime);
  ADC_RegularChannelConfig(ADCSENSOR6_ADC, ADCSENSOR6_CHANNEL, 2, ADDCSENSORSampeTime);

 // Enable DMA request after last transfer (Single-ADC mode) 

  ADC_MultiModeDMARequestAfterLastTransferCmd(ENABLE);
  /*// Enable ADC DMA 
  ADC_DMACmd(ADC1, ENABLE);*/
  
  // Enable ADC3 
  ADC_Cmd(ADC1, ENABLE);
  ADC_Cmd(ADC2, ENABLE);
  ADC_Cmd(ADC3, ENABLE);
  
  // Start the ADC Software Conversion
  ADC_SoftwareStartConv(ADC1);

}

//==============================================================================
// Zugriffsfunktionen
//==============================================================================

//struct ADCData takeOneSet (void)
void takeOneSet(void){
  // Variables
  static uint16_t array[ADCSENSORn][2048];
  // Trigger Sensor
  TRIGGEROn(SENSOR1);
  // Start Measurement  // Sample 2048 Values
  for(int i=0; i<2048; i++){

    array[i][0]=ADCConvertedValue[0];
    array[i][1]=ADCConvertedValue[1];
    array[i][2]=ADCConvertedValue[2];
    array[i][3]=ADCConvertedValue[3];
    array[i][4]=ADCConvertedValue[4];
    array[i][5]=ADCConvertedValue[5];
    }  
  TRIGGEROff(SENSOR1);
  
  // Stop Measurement
  
  // Search Trigger
  
  // Send 1024 Datasets up to the trigger
  
 
}

void getData1(void){
  // Variables
  
  // Trigger Sensor
  TRIGGEROn(SENSOR1);
  for(int j=0;j<0x10000;j++){}
  // Start Measurement  // Sample 2048 Values
  for(int i=0; i<1024; i++){
    dataSet1.ValueSetr[i]=ADCConvertedValue[0];
    dataSet1.i++;
}   
  // Send data
  
  
  TRIGGEROff(SENSOR1);
  USART_send_adc(&dataSet1);
 
  
  
 
}


uint32_t ADC_convertToVoltage(int l){
/* convert the ADC value (from 0 to 0xFFF) to a voltage value (from 0V to 3.3V)*/
   // ADC3ConvertedVoltage = ADC3ConvertedValue *3300/0x0FF;//0xFFF
    ADC3ConvertedVoltage = ADCConvertedValue[l] *3300/0x0FF;//0xFFF
     
    return(ADC3ConvertedVoltage);

}
