/**
  ******************************************************************************
  * @file    i2c.c
  * @author  Fotona d.d.
  * @version V1
  * @date    30-Sept-2013
  * @brief	 I2C communications initialization & ISR
  *
  */ 
	
/** @addtogroup PFM6_Setup
* @{
*/
#include	"pfm.h"
#include	<string.h>
_i2c			*__i2c1;
//______________________________________________________________________________________
void 			Reset_I2C(_i2c *p) {
GPIO_InitTypeDef	GPIO_InitStructure;
					if(p) {
						_SET_ERROR(pfm,_PFM_I2C_ERR);
						GPIO_StructInit(&GPIO_InitStructure);
						GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
						GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
						GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
						GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
						GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
						GPIO_Init(GPIOB, &GPIO_InitStructure);
						GPIO_ResetBits(GPIOB,GPIO_Pin_6 | GPIO_Pin_7);
						_wait(25,_thread_loop);
						I2C_SoftwareResetCmd(I2C1,ENABLE);
						I2C_SoftwareResetCmd(I2C1,DISABLE);
						p=Initialize_I2C(p->addr,p->speed);	
					}
}
//______________________________________________________________________________________
/**
  * @brief  This function initialites the I2C port
  * @param  addr: i2c port address
  * @param  speed:  data transmission speed
  * @retval None
  */
//______________________________________________________________________________________
_i2c 			*Initialize_I2C(int addr, int speed)
{
I2C_InitTypeDef I2C_InitStructure;
GPIO_InitTypeDef	GPIO_InitStructure;
_i2c			*p=calloc(1,sizeof(_i2c));
					if(addr)
						p->addr=addr;
					if(speed)
						p->speed=speed;

					GPIO_StructInit(&GPIO_InitStructure);

					GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
					GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
					GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
					GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
					GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
					GPIO_Init(GPIOB, &GPIO_InitStructure);

					GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_I2C1);
					GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_I2C1);

					RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
					RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C1, ENABLE);
					RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C1, DISABLE); 
					
					I2C_DeInit(I2C1);

					I2C_StructInit(&I2C_InitStructure);
					I2C_InitStructure.I2C_Mode = I2C_Mode_SMBusHost;
					I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
					I2C_InitStructure.I2C_OwnAddress1 = 0x00;
					I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
					I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
					I2C_InitStructure.I2C_ClockSpeed = speed;
					I2C_Init(I2C1, &I2C_InitStructure);

					I2C_StretchClockCmd(I2C1, ENABLE);
					I2C_ARPCmd(I2C1, ENABLE);
					I2C_AcknowledgeConfig(I2C1, ENABLE);	
					I2C_Cmd(I2C1, ENABLE);
					I2C_ITConfig(I2C1, I2C_IT_ERR , ENABLE);

					return p;
}
//______________________________________________________________________________________
/**
  * @brief  This function writes to I2C port, nonblocking 
  * @param  
  * @param  
  * @retval None
  */
//______________________________________________________________________________________
int 			writeI2C( _i2c *p, char *dBuffer, int length) {	
static 
int				nrpt=3;
int				to=__time__+25;
					if(p) {
						memcpy(p->txbuf,dBuffer,length);
						p->ntx=length;
						I2C_ITConfig(I2C1, (I2C_IT_EVT | I2C_IT_BUF), ENABLE);
						I2C_GenerateSTART(I2C1, ENABLE);
						while(p->ntx && __time__< to)
							_thread_loop();
						if(p->ntx) {
							Reset_I2C(p);
							if(--nrpt)
								return	writeI2C(p, dBuffer, length);
						}
						nrpt=3;
						if(p->ntx)
							return(0);					
					}
					_CLEAR_ERROR(pfm,_PFM_I2C_ERR);
					return(-1);
}
//______________________________________________________________________________________
/**
  * @brief  This function reads from I2C port
  * @param 
  * @param  
  * @retval None
  */
//______________________________________________________________________________________
int  			readI2C(_i2c *p, char* dBuffer, int length) {
static
int				nrpt=3;
int				to=__time__+25;

					if(p) {
						p->nrx=length;
						if(dBuffer[1])
							writeI2C(p,dBuffer,2);
						else
							writeI2C(p,dBuffer,1);
						while(p->nrx && __time__< to)
							_thread_loop();
						if(p->nrx) {
							Reset_I2C(p);
							if(--nrpt)
								return	readI2C(p, dBuffer, length);
						}
						nrpt=3;
						memcpy(dBuffer,p->rxbuf,length);
						if(p->nrx)
							return(0);
					} else
						while(length--)
								dBuffer[length]=0;
					_CLEAR_ERROR(pfm,_PFM_I2C_ERR);
						return(-1);
}
//______________________________________________________________________________________
/**
  * @brief  This function handles I2C1 event interrupt request.
  * @param  None
  * @retval None
  */
//______________________________________________________________________________________
void 			I2C1_EV_IRQHandler(void)
{
static 
int 			n=0;

					switch (I2C_GetLastEvent(I2C1)) {	
						
						case I2C_EVENT_MASTER_MODE_SELECT :	
							if(__i2c1->ntx)
								I2C_Send7bitAddress(I2C1, (uint8_t)(__i2c1->addr)<<1, I2C_Direction_Transmitter);
							else
								I2C_Send7bitAddress(I2C1, (uint8_t)(__i2c1->addr)<<1, I2C_Direction_Receiver);
							n=0;
							break;
							
						case I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED:
							I2C_SendData(I2C1, __i2c1->txbuf[n++]);
							break;
							
						case I2C_EVENT_MASTER_BYTE_TRANSMITTING:
						case I2C_EVENT_MASTER_BYTE_TRANSMITTED: 
							if (n == __i2c1->ntx) {
								__i2c1->ntx=0;
								if(__i2c1->nrx)
									I2C_GenerateSTART(I2C1, ENABLE);
								else {
									I2C_GenerateSTOP(I2C1, ENABLE);
									I2C_ITConfig(I2C1, I2C_IT_EVT | I2C_IT_BUF, DISABLE);
								}
							} else
								I2C_SendData(I2C1, __i2c1->txbuf[n++]);
							break;
							
						case I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED:
							if(__i2c1->nrx==1) {
								I2C_AcknowledgeConfig(I2C1, DISABLE);
								I2C_GenerateSTOP(I2C1, ENABLE);
							}
							break;
							
						case I2C_EVENT_MASTER_BYTE_RECEIVED:
							__i2c1->rxbuf[n++]=I2C_ReceiveData (I2C1);
							if(n==__i2c1->nrx-1) {
								I2C_AcknowledgeConfig(I2C1, DISABLE);
								I2C_GenerateSTOP(I2C1, ENABLE);
							}
							if(n==__i2c1->nrx) {	
								I2C_ITConfig(I2C1, I2C_IT_EVT | I2C_IT_BUF, DISABLE);
								I2C_AcknowledgeConfig(I2C1, ENABLE);
								__i2c1->nrx=0;
								}
							break;
							
						default:
							break;
					}
}
//______________________________________________________________________________________
/**
  * @brief  This function handles I2C1 Error interrupt request.
  * @param  None
  * @retval None
  */
//______________________________________________________________________________________
void 			I2C1_ER_IRQHandler(void)
{
					if ((I2C_ReadRegister(I2C1, I2C_Register_SR1) & 0xFF00) != 0x00)
					{
						I2C1->SR1 &= 0x00FF;
					}
}
