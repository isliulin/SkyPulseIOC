/**
  ******************************************************************************
  * @file    serial.c
  * @author  Fotona d.d.
  * @version V1
  * @date    30-Sept-2013
  * @brief	 Serial port initialization & ISR
  *
  */

/** @addtogroup PFM6_Setup
* @{
*/
#if defined  (STM32F2XX)
#include		"stm32f2xx.h"
#elif defined (STM32F10X_HD)
#include		"stm32f10x.h"
#elif	undefined (STM32F2XX || STM32F10X_HD)
*** undefined target !!!!
#endif
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "io.h"

_io	*__com0,*__dbug;
/*******************************************************************************
* Function Name  : DMA_Configuration
* Description    : Configures the DMA.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
#define USART1_DR_Base  USART1_BASE+4
#define RxBufferSize		128
#define TxBufferSize		128
/* Private variables ---------------------------------------------------------*/
USART_InitTypeDef USART_InitStructure;
void DMA_Configuration(_io *io)
{
	DMA_InitTypeDef DMA_InitStructure;
	DMA_StructInit(&DMA_InitStructure);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);

/*DMA Channel4 USART1 TX*/
	DMA_DeInit(DMA2_Stream7);
	DMA_InitStructure.DMA_Channel = DMA_Channel_4;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)USART1_DR_Base;
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)((_buffer *)(io->tx))->_buf;
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
	DMA_InitStructure.DMA_BufferSize = 0;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;

	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
//	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;
//	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;

	DMA_Init(DMA2_Stream7, &DMA_InitStructure);
	DMA_Cmd(DMA2_Stream7, ENABLE);

/*DMA Channel4 USART1 RX*/
	DMA_DeInit(DMA2_Stream2);
	DMA_InitStructure.DMA_Channel = DMA_Channel_4;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)USART1_DR_Base;
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)((_buffer *)(io->rx))->_buf;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_InitStructure.DMA_BufferSize = RxBufferSize;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;

	DMA_Init(DMA2_Stream2, &DMA_InitStructure);
	DMA_Cmd(DMA2_Stream2, ENABLE);

	USART_DMACmd(USART1, USART_DMAReq_Rx | USART_DMAReq_Tx, ENABLE);
}
//______________________________________________________________________________________
volatile int32_t 
		ITM_RxBuffer=ITM_RXBUFFER_EMPTY; 
int	getCOM(_buffer *p) {
int	i=0;
#ifdef __DISCO__
	if(ITM_CheckChar()) {
		i=ITM_ReceiveChar();
		_buffer_push(p,&i,1);
	}		
#else
	if(DMA2_Stream2->NDTR)
		p->_push=&(p->_buf[p->size - DMA2_Stream2->NDTR]);
	else
		p->_push=p->_buf;
#endif
	if(_buffer_pull(p,&i,1))
		return i;
	else
		return EOF;
}
//______________________________________________________________________________________
int	putCOM(_buffer *p, int	c) {
#ifdef __DISCO__
	return	ITM_SendChar(c);
#else
static
	int n=0;

	if(DMA2_Stream7->NDTR==0)																					// end of buffer reached
		n=0;																														// reset static index
	if(n == TxBufferSize-1)																						// buffer full ?
		return(EOF);	
	
	DMA2_Stream7->CR &= ~0x00000001;																	// stream 7 off
	while(DMA2_Stream7->CR & 0x00000001);															// wait for HW

	DMA2_Stream7->M0AR=(uint32_t)(&p->_buf[n-DMA2_Stream7->NDTR]);		// set DMA pointer to new character
	DMA2_Stream7->NDTR++;																							// increase DMA counter
	p->_buf[n++]=c;																										// enter new character
	
	DMA2->HIFCR = 0x0F400000;																					// clear all flags
	DMA2_Stream7->CR |= 0x00000001;																		// stream 7 on
	return(c);
#endif
}
//______________________________________________________________________________________
int	putCOMisr(void *p, int	c) {
int i;
	i=_buffer_push(__com0->tx,&c,1);	
	USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
	return(i);
}
/*******************************************************************************
* Function Name  : USART1_IRQHandler
* Description    : This function handles USART1  interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USART1_IRQHandler(void) {
int i=0;
	if(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) != RESET) {
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
		i=USART_ReceiveData(USART1);
		_buffer_push(__com0->rx,&i,1);
		}
	if(USART_GetFlagStatus(USART1, USART_FLAG_TXE) != RESET) {
		USART_ClearITPendingBit(USART1, USART_IT_TXE);
		if (_buffer_pull(__com0->tx,&i,1))															// if data available
			USART_SendData(USART1, i);																		// send!
		else																														// else
			USART_ITConfig(USART1, USART_IT_TXE, DISABLE);								// disable interrupt
	}
}
//______________________________________________________________________________________
_io *Initialize_USART(int speed) {

USART_InitTypeDef 			USART_InitStructure;
USART_ClockInitTypeDef  USART_ClockInitStructure;
_io 										*io;
#if  defined (__PFM6__)
GPIO_InitTypeDef				GPIO_InitStructure;
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
#elif  defined (__DISCO__)
//	GPIO_InitTypeDef				GPIO_InitStructure;
//	GPIO_StructInit(&GPIO_InitStructure);
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
// 	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
//	GPIO_Init(GPIOB, &GPIO_InitStructure);

//	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
// 	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
//	GPIO_Init(GPIOB, &GPIO_InitStructure);

// 	GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_USART1);		
// 	GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_USART1);	
#else
	#### error, no HW defined
#endif
	io=_io_init(RxBufferSize,TxBufferSize);	// initialize buffer
	io->get= getCOM;	
	io->put= putCOM;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
 
	USART_ClockInitStructure.USART_Clock = USART_Clock_Disable;
	USART_ClockInitStructure.USART_CPOL = USART_CPOL_Low;
	USART_ClockInitStructure.USART_CPHA = USART_CPHA_2Edge;
	USART_ClockInitStructure.USART_LastBit = USART_LastBit_Disable;
	USART_ClockInit(USART1, &USART_ClockInitStructure);
 
	USART_InitStructure.USART_BaudRate = speed;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);

	/* Enable USART1 */
	DMA_Configuration(io);
	USART_Cmd(USART1, ENABLE);
//	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);		
	return io;
}
//______________________________________________________________________________________
//
// zamenjava za gets, ne èaka, vraèa pointer na string brez \r(!!!) ali NULL	
// èe je mode ECHO (-1) na
//							<cr> izpiše <cr><lf>
//							<backspace> ali <del> izpiše <backspace><space><backspace>	
//
//______________________________________________________________________________________
char		*cgets(int c, int mode)
{
_io				*io=__STDIN;
_buffer		*p=io->cmdline;
			
			if(!p)
				p=io->cmdline=_buffer_init(((_buffer *)io->rx)->size);
			switch(c) {
				case EOF:		
					break;
				case '\r':
				case '\n':
					*p->_push = '\0';
					p->_push=p->_pull=p->_buf;
					return(p->_buf);
				case 0x08:
				case 0x7F:
					if(p->_push != p->_pull) {
						--p->_push;
						if(mode)
							printf("\b \b");
					}
					break;
				default:
					if(p->_push != &p->_buf[p->size-1])
						*p->_push++ = c;
					else  {
						*p->_push=c;
						if(mode)
							fputc('\b',&__stdout);
					}
					if(mode) {
						if(isprint(c))
							fputc(c,&__stdout);
						else
							printf("%c%02X%c",'<',c,'>');
					}
					break;
			}
			return(NULL);
}
//______________________________________________________________________________________
int			strscan(char *s,char *ss[],int c)
{
			int		i=0;
			while(1)
			{
				while(*s==' ') ++s;
				if(!*s)
					return(i);

				ss[i++]=s;
				while(*s && *s!=c)
				{
					if(*s==' ')
						*s='\0';
					s++;
				}
				if(!*s)
					return(i);
				*s++=0;
			}
}
//______________________________________________________________________________________
int			hex2asc(int i)
{
			if(i<10)
				return(i+'0');
			else 
				return(i-10+'A');
}
//_____________________________________________________________________________________
int			asc2hex(int i)
{
			if(isxdigit(i))
			{
				if(isdigit(i))
					return(i-'0');
				else
					return(toupper(i)-'A'+0x0a);
			}
			else
				return(0);
}
//______________________________________________________________________________________
int			getHEX(char *p, int n)
{
			int	i=0;
			while(n-- && isxdigit(*p))
				i=(i<<4) | asc2hex(*p++);
			return(i);
}
//______________________________________________________________________________________
void		putHEX(unsigned int i,int n)
{
			if(n>1)
				putHEX(i>>4,n-1);
			fputc(hex2asc(i & 0x0f),&__stdout);
}
//_____________________________________________________________________________________
char		*endstr(char *p)
{
			while(*p)
				++p;
			return(p);
}
//_____________________________________________________________________________________
#define		HEXREC3
//_____________________________________________________________________________________
int			sDump(char *p,int n)
{
			int	i,j;

#ifdef	HEXREC1
			i=(int)p + ((int)p >> 8);
			if(n<252)
				j=n;
			else
				j=252;
			n -= j;
			i += (j+3);
			printf("\r\nS1%02X%04X",j+3,(int)p);
#endif

#ifdef	HEXREC2
			i=(int)p + ((int)p >> 8) + ((int)p >> 16);
			if(n<250)
				j=n;
			else
				j=250;
			n -= j;
			i += (j+4);
			printf("\r\nS2%02X%06X",j+4,(int)p);
#endif

#ifdef	HEXREC3
			i=(int)p + ((int)p >> 8)+ ((int)p >>16)+ ((int)p >> 24);
			if(n<248)
				j=n;
			else
				j=248;
			n -= j;
			i += (j+5);
			printf("\r\nS3%02X%08X",j+5,(int)p);
#endif
//_____________________________________________________________________________________
			while(j--)
			{
				i += *p;
				putHEX(*p++,2);
			}
			putHEX(~i,2);
			if(n)
				sDump(p,n);
			return(-1);
}
//_____________________________________________________________________________________
int			iDump(int *p,int n)
{
			int		i,j,k;
			union 	{int i; char c[sizeof(int)];} u;

#ifdef	HEXREC1
			i=(int)p + ((int)p >> 8);
			if(n < (255-3)/sizeof(int))
				j=n;
			else
				j=(255-3)/sizeof(int);
			n -= j;
			i += (sizeof(int)*j+3);
			printf("\r\nS1%02X%04X",sizeof(int)*j+3,(int)p);
#endif

#ifdef	HEXREC2
			i=(int)p + ((int)p >> 8) + ((int)p >> 16);
			if(n < (255-5)/sizeof(int))
				j=n;
			else
				j=(255-5)/sizeof(int);
			n -= j;
			i += (sizeof(int)*j+4);
			printf("\r\nS2%02X%06X",sizeof(int)*j+4,(int)p);
#endif

#ifdef	HEXREC3
			i=(int)p + ((int)p >> 8)+ ((int)p >>16)+ ((int)p >> 24);
			if(n < (255-7)/sizeof(int))
				j=n;
			else
				j=(255-7)/sizeof(int);
			n -= j;
			i += (sizeof(int)*j+5);
			printf("\r\nS3%02X%08X",sizeof(int)*j+5,(int)p);
#endif
//_____________________________________________________________________________________
			while(j--)
			{
				u.i=*p++;
				for(k=0; k<sizeof(int); ++k)
				{
					i += u.c[k]; 
					putHEX(u.c[k] ,2);
				}
			}
			putHEX(~i,2);
			if(n)
				iDump(p,n);
			return(-1);
}
	int 	err,i,n,a;
//_____________________________________________________________________________________
int			sLoad(char *p)
{
	int k=0;
	if(!strncmp(p,"S1",2)) {
		n=getHEX(&p[2],2);
		a=getHEX(&p[4],4);
		k=2;
	}
	if(!strncmp(p,"S2",2)) {
		n=getHEX(&p[2],2);
		a=getHEX(&p[4],6);
		k=3;
	}
	if(!strncmp(p,"S3",2)) {
		n=getHEX(&p[2],2);
		a=getHEX(&p[4],8);
		k=4;
	}
	if(!k)
		return(-1);
	err=0;
	for(i=0;i<n+1;++i)
		err += getHEX(&p[2*i+2],2);
	if(++err & 0xff)
		return(-1);
	else
		for(i=0;i<n-k-1;++i)
			((char *)a)[i]= getHEX(&p[2*i+2*k+4],2);
	return 0;
}
/**
* @}
*/ 
