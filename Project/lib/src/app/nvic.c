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
}
/******************************************************************************/
void	Watchdog_init(int t) {
#ifdef	__PFM6__
			IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
			IWDG_SetPrescaler(IWDG_Prescaler_32);
			IWDG_SetReload(t);
			while(IWDG_GetFlagStatus(IWDG_FLAG_RVU) == RESET);
			IWDG_ReloadCounter();
			IWDG_Enable();
			IWDG_WriteAccessCmd(IWDG_WriteAccess_Disable);
#endif
}
/******************************************************************************/
void	Watchdog(void) {
#ifdef	__PFM6__
			IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
			IWDG_ReloadCounter();
			IWDG_WriteAccessCmd(IWDG_WriteAccess_Disable);
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
