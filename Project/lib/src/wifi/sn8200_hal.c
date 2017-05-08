#include	"stm32f2xx.h"
#include	<stdbool.h>
#include	"io.h"
#include	"sn8200_api.h"
#include	"sn8200_core.h"
#include	"delay.h"

extern
_io		*__com0;
_io	  *__hal;
int		halRx=-1;

int		__new_put(_buffer *tx, int c) {
			return(_buffer_push(__com0->tx,&c,1));
}

int		__new_get(_buffer *rx) {
int		i=0;
			if(_buffer_pull(__com0->rx,&i,1))
				return(i);
			else
				return(EOF);
}
	
void	SN8200_HAL_Init(uint32_t baudrate)
{
			if(!__hal) {
				__hal=__com0;
				__com0=_io_init(100,5000);
				__com0->put=__new_put;
				__com0->get=__new_get;	
				if(__stdin.handle.io==__hal)
					_stdio(__com0);
			}
}

void	SN8200_HAL_SendData(unsigned char *buf, int len)
{
			while(DMA2_Stream7->NDTR)
				mdelay(1);
			
			DMA2_Stream7->CR &= ~0x00000001;																			// stream 7 off
			while(DMA2_Stream7->CR & 0x00000001);
			
			DMA2_Stream7->NDTR=len;																								// increase DMA counter
			DMA2_Stream7->M0AR=(uint32_t)(buf);		

			DMA2->HIFCR = 0x0F400000;																							// clear all flags
			DMA2_Stream7->CR |= 0x00000001;	
			
			while(DMA2_Stream7->NDTR)
				mdelay(1);
}

bool	SN8200_HAL_RxBufferEmpty(void)
{
			if(halRx != -1)
				return false;
			halRx=__hal->get(__hal->rx);
			if(halRx != -1)		
				return false;
			else 
				return true;
}

uint8_t SN8200_HAL_ReadByte(void)
{
int 	i=halRx;
			halRx=-1;
			return i;
}
