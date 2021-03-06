/**
  ******************************************************************************
  * @file			can.cpp
  * @author		Fotona d.d.
  * @version 
  * @date    
  * @brief		CAN initialization
  *
  */
/** @addtogroup
* @{
*/
#include	"can.h"
#include	"lm.h"
#include	"string.h"
/*******************************************************************************/
static		_CAN	*me=NULL;
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
_CAN			*_CAN::Instance(void) {
					return me;	
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
_CAN::_CAN(bool loopback) {
CAN_InitTypeDef					CAN_InitStructure;
CAN_FilterInitTypeDef		CAN_FilterInitStructure;
NVIC_InitTypeDef 				NVIC_InitStructure;
				if(me==NULL) {
#ifndef __SIMULATION__					
GPIO_InitTypeDef				GPIO_InitStructure;
					GPIO_StructInit(&GPIO_InitStructure);
					GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
					GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	
					GPIO_InitStructure.GPIO_Pin = 1<<CAN_RXPIN;
					GPIO_Init(CAN_GPIO, &GPIO_InitStructure);
					GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
					GPIO_InitStructure.GPIO_Pin = 1<<CAN_TXPIN;
					GPIO_Init(CAN_GPIO, &GPIO_InitStructure);

					GPIO_PinAFConfig(CAN_GPIO, CAN_RXPIN, GPIO_AF_CAN);
					GPIO_PinAFConfig(CAN_GPIO, CAN_TXPIN, GPIO_AF_CAN);
#endif
					RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);							// glej opis driverja, �mafu, treba inicializirat komplet :(
					RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN2, ENABLE);
					CAN_StructInit(&CAN_InitStructure);
					CAN_DeInit(__CAN__);

					CAN_InitStructure.CAN_TTCM=DISABLE;
					CAN_InitStructure.CAN_ABOM=ENABLE;
					CAN_InitStructure.CAN_AWUM=DISABLE;
					CAN_InitStructure.CAN_NART=DISABLE;
					CAN_InitStructure.CAN_RFLM=DISABLE;

//... pomembn.. da ne zamesa mailboxov in jih oddaja po vrstnem redu vpisovanja... ni default !!!
					CAN_InitStructure.CAN_TXFP=ENABLE;	
//... prijava instance za ISR
					io=_io_init(100*sizeof(CanRxMsg),100*sizeof(CanTxMsg));
					me=this;
					
					if(loopback)
						CAN_InitStructure.CAN_Mode=CAN_Mode_LoopBack;
					else
						CAN_InitStructure.CAN_Mode=CAN_Mode_Normal;

					CAN_InitStructure.CAN_SJW=CAN_SJW_4tq;
					CAN_InitStructure.CAN_BS1=CAN_BS1_10tq;
					CAN_InitStructure.CAN_BS2=CAN_BS2_4tq;
					CAN_InitStructure.CAN_Prescaler=4;
					CAN_Init(__CAN__,&CAN_InitStructure);

					CAN_FilterInitStructure.CAN_FilterMode=CAN_FilterMode_IdMask;
					CAN_FilterInitStructure.CAN_FilterScale=CAN_FilterScale_32bit;
					CAN_FilterInitStructure.CAN_FilterActivation=ENABLE;
					CAN_FilterInitStructure.CAN_FilterFIFOAssignment=CAN_FIFO0;

					CAN_FilterInitStructure.CAN_FilterIdHigh=idIOC_State<<5;
					CAN_FilterInitStructure.CAN_FilterMaskIdHigh=0x7f0<<5;
					CAN_FilterInitStructure.CAN_FilterIdLow = 0;
					CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0;
					CAN_FilterInitStructure.CAN_FilterNumber=__FILT_BASE__+0;
					CAN_FilterInit(&CAN_FilterInitStructure);

					CAN_FilterInitStructure.CAN_FilterIdHigh=idIOC_State_Ack<<5;
					CAN_FilterInitStructure.CAN_FilterMaskIdHigh=0x7f0<<5;
					CAN_FilterInitStructure.CAN_FilterNumber=__FILT_BASE__+1;
					CAN_FilterInit(&CAN_FilterInitStructure);

					CAN_FilterInitStructure.CAN_FilterIdHigh=idCAN2COM<<5;
					CAN_FilterInitStructure.CAN_FilterMaskIdHigh=0x7f0<<5;
					CAN_FilterInitStructure.CAN_FilterNumber=__FILT_BASE__+2;
					CAN_FilterInit(&CAN_FilterInitStructure);

					NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
					NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
					NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
					NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
						
					NVIC_InitStructure.NVIC_IRQChannel = CAN1_RX0_IRQn;
					NVIC_Init(&NVIC_InitStructure);
					NVIC_InitStructure.NVIC_IRQChannel = CAN2_RX0_IRQn;
					NVIC_Init(&NVIC_InitStructure);

					NVIC_InitStructure.NVIC_IRQChannel = CAN1_TX_IRQn;
					NVIC_Init(&NVIC_InitStructure);
					NVIC_InitStructure.NVIC_IRQChannel = CAN2_TX_IRQn;
					NVIC_Init(&NVIC_InitStructure);
					
					CAN_ITConfig(__CAN__, CAN_IT_FMP0, ENABLE);
					com=NULL;
				}
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void			_CAN::ISR_rx() {
CanRxMsg	buf;
					CAN_Receive(__CAN__, CAN_FIFO0, &buf);
					_buffer_push(me->io->rx,&buf,sizeof(CanRxMsg));
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void			_CAN::ISR_tx() {
CanTxMsg	buf={0,0,CAN_ID_STD,CAN_RTR_DATA,0,0,0,0,0,0,0,0,0};
					if(_buffer_pull(me->io->tx,&buf,sizeof(CanTxMsg)))
						CAN_Transmit(__CAN__,&buf);									// oddaj, ce je kaj na bufferju
					else
						CAN_ITConfig(__CAN__, CAN_IT_TME, DISABLE);	// sicer zapri interrupt				
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void			_CAN::Send(CanTxMsg *msg) {
					if(_buffer_count(io->tx) > 0 || (__CAN__->TSR & CAN_TSR_TME) == 0) {
						_buffer_push(io->tx,&msg,sizeof(msg));
					} else
						CAN_Transmit(__CAN__,msg);
					CAN_ITConfig(__CAN__, CAN_IT_TME, ENABLE);		
//________ debug print__________________________________________________________________
					if(_BIT(_LM::debug, DBG_CAN_TX)) {
//						_io *temp=_stdio(lm->io);
						printf("\r\n:%04d>%02X ",__time__ % 10000,msg->StdId);
						for(int i=0;i<msg->DLC;++i)
							printf(" %02X",msg->Data[i]);
						printf("\r\n:");
//						_stdio(temp);
					}
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void			_CAN::Send(char *msg) {	
CanTxMsg	buf={0,0,CAN_ID_STD,CAN_RTR_DATA,0,0,0,0,0,0,0,0,0};
					buf.StdId=strtol(msg,&msg,16);
					do {
						while(*msg == ' ') ++msg;
						for(buf.DLC=0; *msg && buf.DLC < 8; ++buf.DLC)
							buf.Data[buf.DLC]=strtol(msg,&msg,16);
						Send(&buf);
					} while(*msg);
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void			_CAN::Recv(char *msg) {	
CanRxMsg	buf={0,0,CAN_ID_STD,CAN_RTR_DATA,0,0,0,0,0,0,0,0,0};
					buf.StdId=strtol(msg,&msg,16);
					do {
						while(*msg == ' ') ++msg;
						for(buf.DLC=0; *msg && buf.DLC < 8; ++buf.DLC)
							buf.Data[buf.DLC]=strtol(msg,&msg,16);
						CAN_ITConfig(__CAN__, CAN_IT_FMP0, DISABLE);
						_buffer_push(io->rx,&buf,sizeof(buf));
						CAN_ITConfig(__CAN__, CAN_IT_FMP0, ENABLE);
					} while(*msg);
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void			_CAN::Console(void *v) {	
_LM				*lm = (_LM *)v;
					if(lm->can.com) {
_io*				io=_stdio(lm->can.com);
						lm->Parse(getchar());
						_stdio(io);			
					}
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void			_CAN::Parse(void *v) {	
_LM				*lm = (_LM *)v;
CanRxMsg	rxm={0,0,CAN_ID_STD,CAN_RTR_DATA,0,0,0,0,0,0,0,0,0};		
//________ flushing com buffer/not echoed if debug_________ 
						if(com && io->tx->size - _buffer_count(io->tx) > sizeof(CanTxMsg)) {
CanTxMsg			txm={0,0,CAN_ID_STD,CAN_RTR_DATA,0,0,0,0,0,0,0,0,0};		
							txm.StdId=idCOM2CAN;
							txm.DLC=_buffer_pull(com->tx,txm.Data,sizeof(txm.Data));
							if(txm.DLC)
								Send(&txm);
						}
//______________________________________________________________________________________					
					if(_buffer_count(io->rx) && _buffer_pull(io->rx,&rxm,sizeof(CanTxMsg))) {
//
//________ debug print__________________________________________________________________
						if(_BIT(_LM::debug, DBG_CAN_RX)) {
//							_io *temp=_stdio(lm->io);
							printf("\r\n:%04d<%02X ",__time__ % 10000,rxm.StdId);
							for(int i=0;i<rxm.DLC;++i)
								printf(" %02X",rxm.Data[i]);
							printf("\r\n:");
//							_stdio(temp);
						}
//______________________________________________________________________________________
						switch(rxm.StdId) {
							case idIOC_State:
								if(rxm.DLC)
									lm->IOC_State.State=(_State)rxm.Data[0];
								lm->IOC_State.Send();
								break;
							case idIOC_Cmd:
								lm->spray.AirLevel 		= __min(rxm.Data[0],10);
								lm->spray.WaterLevel 	= __min(rxm.Data[1],10);
							break;
//______________________________________________________________________________________
							case idCAN2COM:
								if(rxm.DLC) {
									if(com == NULL) {
										com = _io_init(128,128);
										_thread_add((void *)Console,lm,(char *)"CAN console",0);
									}
									_buffer_push(com->rx,rxm.Data,rxm.DLC);
								} else {
								_thread_remove((void *)Console,lm);
									com=_io_close(com);
								}
								break;
//______________________________________________________________________________________							
							case idCOM2CAN:
							{
_io*							io=_stdio(lm->io);
									for(int i=0; i<rxm.DLC; ++i)
										putchar(rxm.Data[i]);
									_stdio(io);
							}
							break;
//______________________________________________________________________________________					
							default:
							break;
						}
					}
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
extern 		"C" {
void 			CAN1_TX_IRQHandler() {
					me->ISR_tx();
}
void 			CAN2_TX_IRQHandler() {
					me->ISR_tx();
}
void 			CAN1_RX0_IRQHandler() {
					me->ISR_rx();
}
void 			CAN2_RX0_IRQHandler() {
					me->ISR_rx();
}
void 			CAN1_SCE_IRQHandler(void) {
					CAN1->MSR |= 0x0004;
					}
void 			CAN2_SCE_IRQHandler(void) {
					CAN2->MSR |= 0x0004;
					}
}
/**
* @}
*/


