/**
  ******************************************************************************
  * @file    nvic.c
  * @author  Fotona d.d.
  * @version V1
  * @date    30-Sept-2013
  * @brief	 Interrupt controller initialization 
  *
  */ 

/** @addtogroup PFM6_Setup
* @brief PFM6 peripherals setup group
* @{
*/
#include	"app.h"
/*******************************************************************************
* Function Name  : InterruptConfig
* Description    : Configures the used IRQ Channels and sets their priority.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void 	Initialize_NVIC() {
			NVIC_InitTypeDef NVIC_InitStructure;
			NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);  				 		/* Configure the Priority Group to 2 bits */
			NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
			NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
			NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
			
			NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_TIM10_IRQn; 		// pulse generator, 1. priority
			NVIC_Init(&NVIC_InitStructure);
			
			NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;

			NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream4_IRQn; 		// pulse recording, 2. priority
			NVIC_Init(&NVIC_InitStructure);
			
			NVIC_InitStructure.NVIC_IRQChannel = I2C1_EV_IRQn;					// I2C
			NVIC_Init(&NVIC_InitStructure);
			
			NVIC_InitStructure.NVIC_IRQChannel = I2C1_ER_IRQn;
			NVIC_Init(&NVIC_InitStructure);
			
			NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
				
			NVIC_InitStructure.NVIC_IRQChannel = CAN1_RX0_IRQn;					// CAN
			NVIC_Init(&NVIC_InitStructure);
			
			NVIC_InitStructure.NVIC_IRQChannel = CAN2_RX0_IRQn;
			NVIC_Init(&NVIC_InitStructure);

			NVIC_InitStructure.NVIC_IRQChannel = CAN1_TX_IRQn;					// CAN
			NVIC_Init(&NVIC_InitStructure);
			
			NVIC_InitStructure.NVIC_IRQChannel = CAN2_TX_IRQn;
			NVIC_Init(&NVIC_InitStructure);

			NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;				// HW errors
			NVIC_Init(&NVIC_InitStructure);
			
			NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
			NVIC_Init(&NVIC_InitStructure);
			
			NVIC_InitStructure.NVIC_IRQChannel = ADC_IRQn;							// analog watchdogs
			NVIC_Init(&NVIC_InitStructure);

			NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;							// fan sensor
			NVIC_Init(&NVIC_InitStructure);

			NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;	
			NVIC_Init(&NVIC_InitStructure);

			NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;						// USART1
			NVIC_Init(&NVIC_InitStructure);

			NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;						// USART1
			NVIC_Init(&NVIC_InitStructure);
}
/******************************************************************************/
#define LSE_STARTUP_TIMEOUT 500
void	Rtc_init() {
			uint32_t StartUpCounter = 0;
			uint32_t LSEStatus = 0;
			uint32_t rtc_freq = 0;

			RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
			PWR_BackupAccessCmd(ENABLE);
//			RCC_BackupResetCmd(ENABLE);
//			RCC_BackupResetCmd(DISABLE);
			RCC_LSEConfig(RCC_LSE_ON);
	
			do {
					LSEStatus = RCC_GetFlagStatus(RCC_FLAG_LSERDY);
					_wait(1,_thread_loop);
					StartUpCounter++;
			} while ((LSEStatus == 0) && (StartUpCounter <= LSE_STARTUP_TIMEOUT));

			if (StartUpCounter > LSE_STARTUP_TIMEOUT) {
				RCC_LSEConfig(RCC_LSE_OFF);
				RCC_LSICmd(ENABLE);
				while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET) {} 
					RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
					rtc_freq = 40000;
			} else {
					RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
					rtc_freq = 32768;
			}
			RCC_RTCCLKCmd(ENABLE);
			RTC_WaitForSynchro();
			RTC_InitTypeDef RTC_InitStructure;
			RTC_InitStructure.RTC_AsynchPrediv = 127;
			RTC_InitStructure.RTC_SynchPrediv    = (rtc_freq / 128) - 1;
			RTC_InitStructure.RTC_HourFormat   = RTC_HourFormat_24;
			RTC_Init(&RTC_InitStructure);
			
			PWR_BackupAccessCmd(DISABLE); 
}
/******************************************************************************/
time_t	rtc_read(void) {
			RTC_DateTypeDef dateStruct;
			RTC_TimeTypeDef timeStruct;
			struct tm timeinfo;
			RTC_GetTime(RTC_Format_BIN, &timeStruct);
			RTC_GetDate(RTC_Format_BIN, &dateStruct);
			timeinfo.tm_wday = dateStruct.RTC_WeekDay;
			timeinfo.tm_mon  = dateStruct.RTC_Month - 1;
			timeinfo.tm_mday = dateStruct.RTC_Date;
			timeinfo.tm_year = dateStruct.RTC_Year + 100;
			timeinfo.tm_hour = timeStruct.RTC_Hours;
			timeinfo.tm_min  = timeStruct.RTC_Minutes;
			timeinfo.tm_sec  = timeStruct.RTC_Seconds;
			time_t t = mktime(&timeinfo);
			
			return t;    
}
/******************************************************************************/
void	rtc_write(time_t t) {
			RTC_DateTypeDef dateStruct;
			RTC_TimeTypeDef timeStruct;
			struct tm *timeinfo = localtime(&t);
			dateStruct.RTC_WeekDay = timeinfo->tm_wday;
			dateStruct.RTC_Month   = timeinfo->tm_mon + 1;
			dateStruct.RTC_Date    = timeinfo->tm_mday;
			dateStruct.RTC_Year    = timeinfo->tm_year - 100;
			timeStruct.RTC_Hours   = timeinfo->tm_hour;
			timeStruct.RTC_Minutes = timeinfo->tm_min;
			timeStruct.RTC_Seconds = timeinfo->tm_sec;
			timeStruct.RTC_H12     = RTC_HourFormat_24;
			PWR_BackupAccessCmd(ENABLE);   
			RTC_SetDate(RTC_Format_BIN, &dateStruct);
			RTC_SetTime(RTC_Format_BIN, &timeStruct);    
			PWR_BackupAccessCmd(DISABLE);
}
/******************************************************************************/
void	Watchdog_init(int t) {
#ifdef	__PFM6__
//			IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
//			IWDG_SetPrescaler(IWDG_Prescaler_32);
//			IWDG_SetReload(t);
//			while(IWDG_GetFlagStatus(IWDG_FLAG_RVU) == RESET);
//			IWDG_ReloadCounter();
//			IWDG_Enable();
//			IWDG_WriteAccessCmd(IWDG_WriteAccess_Disable);
#endif
}
/******************************************************************************/
void	Watchdog(void) {
#ifdef	__PFM6__
//			IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
//			IWDG_ReloadCounter();
//			IWDG_WriteAccessCmd(IWDG_WriteAccess_Disable);
#endif
}
/******************************************************************************/
void	WWDG_init(void) {
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_WWDG, ENABLE);
			WWDG_SetPrescaler(WWDG_Prescaler_8);											/* WWDG clock counter = (PCLK1/4096)/8 = 244 Hz (~4 ms)  */
			WWDG_SetWindowValue(0x41);
			WWDG_Enable(0x7F);																				/* Enable WWDG and set counter value to 0x7F, WWDG timeout = ~4 ms * 64 = 262 ms */
}
/******************************************************************************/
void SysTick_init(void)
{
			RCC_ClocksTypeDef RCC_Clocks;
			RCC_GetClocksFreq(&RCC_Clocks);
			SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);
			SysTick_Config(RCC_Clocks.HCLK_Frequency / 1000);
			NVIC_SetPriority(SysTick_IRQn, 0x03);
}
/**
* @}
*/ 
