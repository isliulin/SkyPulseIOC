#include		"iap.h"
#include		<stdio.h>
#include 		<ctype.h>


#define			USART1_DR_Base  USART1_BASE+4
#define 		RxBufferSize		512
#define 		TxBufferSize		512

_io*				__this_io;

#if  defined (__PFM6__) ||  defined (__PFM8__) ||  defined (__DISCO__)
//______________________________________________________________________________________
#ifdef __DISCO__
volatile int32_t 
						ITM_RxBuffer=ITM_RXBUFFER_EMPTY;
						
int					__get(_buffer *rx) {
						int	i;
						if(ITM_CheckChar()) {
							i=ITM_ReceiveChar();
							_buffer_push(rx,&i,1);
						}		
#else						
int					__get(_buffer *rx) {
int					i=0;
						if(DMA2_Stream2->NDTR)
							rx->_push=&(rx->_buf[rx->len - DMA2_Stream2->NDTR]);
						else
							rx->_push=rx->_buf;
#endif
						if(_buffer_pull(rx,&i,1))
							return i;
						else
							return EOF;
}
//______________________________________________________________________________________
int					__put(_buffer *tx, int	c) {
#ifdef __DISCO__
						return	ITM_SendChar(c);
}
#else	
static			int n=0;

						if(DMA2_Stream7->NDTR==0)																					// end of buffer reached
							n=0;																														// reset static index
						if(n == TxBufferSize-1)																						// buffer full ?
							return(EOF);	
						
						DMA2_Stream7->CR &= ~0x00000001;																	// stream 7 off
						while(DMA2_Stream7->CR & 0x00000001);															// wait for HW

						DMA2_Stream7->M0AR=(uint32_t)(&tx->_buf[n-DMA2_Stream7->NDTR]);		// set DMA pointer to new character
						DMA2_Stream7->NDTR++;																							// increase DMA counter
						tx->_buf[n++]=c;																									// enter new character
						
						DMA2->HIFCR = 0x0F400000;																					// clear all flags
						DMA2_Stream7->CR |= 0x00000001;																		// stream 7 on
						return(c);
}
#endif
void 				DMA_Configuration(_io *io)
{
						DMA_InitTypeDef DMA_InitStructure;
						DMA_StructInit(&DMA_InitStructure);
						
						RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);

						DMA_DeInit(DMA2_Stream7);
						DMA_InitStructure.DMA_Channel = DMA_Channel_4;
						DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)USART1_DR_Base;
						DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)(io->tx->_buf);
						DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
						DMA_InitStructure.DMA_BufferSize = 0;
						DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;

						DMA_InitStructure.DMA_Priority = DMA_Priority_High;
						DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
						DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
						DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
						DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;

						DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
						DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;

						DMA_Init(DMA2_Stream7, &DMA_InitStructure);
						DMA_Cmd(DMA2_Stream7, ENABLE);

						DMA_DeInit(DMA2_Stream2);
						DMA_InitStructure.DMA_Channel = DMA_Channel_4;
						DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)USART1_DR_Base;
						DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)(io->rx->_buf);
						DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
						DMA_InitStructure.DMA_BufferSize = RxBufferSize;
						DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;

						DMA_Init(DMA2_Stream2, &DMA_InitStructure);
						DMA_Cmd(DMA2_Stream2, ENABLE);

						USART_DMACmd(USART1, USART_DMAReq_Rx | USART_DMAReq_Tx, ENABLE);
}
#endif
//______________________________________________________________________________________
_io*	 			Initialize_USART(void) {
						USART_InitTypeDef 			USART_InitStructure;
						GPIO_InitTypeDef 				GPIO_InitStructure;
						
						USART_ClockInitTypeDef  USART_ClockInitStructure;

						__this_io=_io_init(RxBufferSize,TxBufferSize);
						__this_io->put= __put;
						__this_io->get= __get;	


#if  defined (__PFM6__) || defined (__PFM8__)
						RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
						RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);
						
						GPIO_StructInit(&GPIO_InitStructure);
						GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
						GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
						GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
						GPIO_Init(GPIOA, &GPIO_InitStructure);

						GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
						GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
						GPIO_Init(GPIOA, &GPIO_InitStructure);

						GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);		
						GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);		
						DMA_Configuration(__this_io);

#elif  defined (__DISCO__)
						RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
						RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);

						GPIO_StructInit(&GPIO_InitStructure);
						GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
						GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
						GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
						GPIO_Init(GPIOB, &GPIO_InitStructure);

						GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
						GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
						GPIO_Init(GPIOB, &GPIO_InitStructure);

						GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_USART1);		
						GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_USART1);	
						DMA_Configuration(__this_io);
						
#else
#### error, no HW defined
#endif				 
						USART_ClockInitStructure.USART_Clock = USART_Clock_Disable;
						USART_ClockInitStructure.USART_CPOL = USART_CPOL_Low;
						USART_ClockInitStructure.USART_CPHA = USART_CPHA_2Edge;
						USART_ClockInitStructure.USART_LastBit = USART_LastBit_Disable;
						USART_ClockInit(USART1, &USART_ClockInitStructure);
					 
						USART_InitStructure.USART_BaudRate = 921600;
						USART_InitStructure.USART_WordLength = USART_WordLength_8b;
						USART_InitStructure.USART_StopBits = USART_StopBits_1;
						USART_InitStructure.USART_Parity = USART_Parity_No;
						USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
						USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
						USART_Init(USART1, &USART_InitStructure);

						USART_Cmd(USART1, ENABLE);
						return __this_io;
}
/*******************************************************************************
* Function Name  : USART1_IRQHandler
* Description    : This function handles USART1  interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void 				USART1_IRQHandler(void) {
int					i;
						if(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) != RESET) {
							i= USART_ReceiveData(USART1);
							USART_ClearITPendingBit(USART1, USART_IT_RXNE);
							_buffer_push(__this_io->rx,&i,1);
						}
						if(USART_GetFlagStatus(USART1, USART_FLAG_TXE) != RESET) {
							USART_ClearITPendingBit(USART1, USART_IT_TXE);
							i=0;																																		// tx ready to send
							if(_buffer_pull(__this_io->tx,&i,1))																		// if data available
								USART_SendData(USART1, i);																						// send!
							else																																		// else
								USART_ITConfig(USART1, USART_IT_TXE, DISABLE);												// disable interrupt
						}
}
/*******************************************************************************
* Function Name  : IO retarget 
* Description    : This function handles USART1  interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
int 				fputc(int c, FILE *f) {
						return	__this_io->put(__this_io->tx,c);
}
//_________________________________________________________________________________
int 				fgetc(FILE *f) {
						return	__this_io->get(__this_io->rx);
}
