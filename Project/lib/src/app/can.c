/**
  ******************************************************************************
  * @file    can.c
  * @author  Fotona d.d.
  * @version V1
  * @date    30-Sept-2013
  * @brief   CAN initialization
  *
  */
	
/** @addtogroup PFM6_Setup
* @{
*/
#include	"pfm.h"
_io				*__can;
/*******************************************************************************
* Function Name  : CAN_Initialize
* Description    : Configures the CAN, transmit and receive using interrupt.
* Input          : None
* Output         : None
* Return         : PASSED if the reception is well done, FAILED in other case
*******************************************************************************/
_io			 	*Initialize_CAN(int loop)
{
CAN_InitTypeDef					CAN_InitStructure;
CAN_FilterInitTypeDef		CAN_FilterInitStructure;
GPIO_InitTypeDef				GPIO_InitStructure;

					GPIO_StructInit(&GPIO_InitStructure);
					GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
					GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

					GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
					GPIO_Init(GPIOB, &GPIO_InitStructure);
					GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
					GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
					GPIO_Init(GPIOB, &GPIO_InitStructure);

					GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_CAN2);
					GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_CAN2);

					RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);							// glej opis driverja, šmafu, treba inicializirat c
					RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN2, ENABLE);
					CAN_StructInit(&CAN_InitStructure);
					CAN_DeInit(__CAN__);

					CAN_InitStructure.CAN_TTCM=DISABLE;
					CAN_InitStructure.CAN_ABOM=ENABLE;
					CAN_InitStructure.CAN_AWUM=DISABLE;
					CAN_InitStructure.CAN_NART=ENABLE;
					CAN_InitStructure.CAN_RFLM=DISABLE;
//... pomembn.. da ne zamesa mailboxov in jih oddaja po vrstnem redu vpisovanja... ni default !!!
					CAN_InitStructure.CAN_TXFP=ENABLE;	

					if(loop)
						CAN_InitStructure.CAN_Mode=CAN_Mode_LoopBack;
					else
						CAN_InitStructure.CAN_Mode=CAN_Mode_Normal;

					CAN_InitStructure.CAN_SJW=CAN_SJW_4tq;
					CAN_InitStructure.CAN_BS1=CAN_BS1_10tq;
					CAN_InitStructure.CAN_BS2=CAN_BS2_4tq;
					CAN_InitStructure.CAN_Prescaler=4;
					CAN_Init(__CAN__,&CAN_InitStructure);

					CAN_FilterInitStructure.CAN_FilterMode=CAN_FilterMode_IdList;
					CAN_FilterInitStructure.CAN_FilterScale=CAN_FilterScale_32bit;
					CAN_FilterInitStructure.CAN_FilterMaskIdLow=0;
					CAN_FilterInitStructure.CAN_FilterIdLow=0;
					CAN_FilterInitStructure.CAN_FilterActivation=ENABLE;

					CAN_FilterInitStructure.CAN_FilterFIFOAssignment=CAN_FIFO0;

	// filtri za PFM in EC
					CAN_FilterInitStructure.CAN_FilterIdHigh=_ID_SYS2PFM<<5;
					CAN_FilterInitStructure.CAN_FilterMaskIdHigh=_ID_SYS2EC<<5;
					CAN_FilterInitStructure.CAN_FilterNumber=__FILT_BASE__+0;
					CAN_FilterInit(&CAN_FilterInitStructure);
					CAN_FilterInitStructure.CAN_FilterIdHigh=_ID_SYS2PFMcom<<5;
					CAN_FilterInitStructure.CAN_FilterMaskIdHigh=_ID_PFMcom2SYS<<5;
					CAN_FilterInitStructure.CAN_FilterNumber=__FILT_BASE__+1;
					CAN_FilterInit(&CAN_FilterInitStructure);
					CAN_FilterInitStructure.CAN_FilterIdHigh=_ID_SYS_TRIGG<<5;
					CAN_FilterInitStructure.CAN_FilterMaskIdHigh=0<<5;
					CAN_FilterInitStructure.CAN_FilterNumber=__FILT_BASE__+2;
					CAN_FilterInit(&CAN_FilterInitStructure);

// filtri za IAP mode
//					CAN_FilterInitStructure.CAN_FilterIdHigh=_ID_IAP_GO<<5;
//					CAN_FilterInitStructure.CAN_FilterMaskIdHigh=_ID_IAP_ADDRESS<<5;
//					CAN_FilterInitStructure.CAN_FilterNumber=__FILT_BASE__+3;
//					CAN_FilterInit(&CAN_FilterInitStructure);

//					CAN_FilterInitStructure.CAN_FilterIdHigh=_ID_IAP_DWORD<<5;
//					CAN_FilterInitStructure.CAN_FilterMaskIdHigh=_ID_IAP_ERASE<<5;
//					CAN_FilterInitStructure.CAN_FilterNumber=__FILT_BASE__+4;
//					CAN_FilterInit(&CAN_FilterInitStructure);

//					CAN_FilterInitStructure.CAN_FilterIdHigh=_ID_IAP_ACK<<5;
//					CAN_FilterInitStructure.CAN_FilterMaskIdHigh=0<<5;
//					CAN_FilterInitStructure.CAN_FilterNumber=__FILT_BASE__+5;
//					CAN_FilterInit(&CAN_FilterInitStructure);
					
					CAN_ITConfig(__CAN__, CAN_IT_FMP0, ENABLE);
					return(_io_init(100*sizeof(CanRxMsg),100*sizeof(CanTxMsg)));
}
/*******************************************************************************/
void 			CAN1_TX_IRQHandler(void)
{
CanTxMsg	tx;
					if(_buffer_pull(__can->tx,&tx,sizeof(CanTxMsg)))
						CAN_Transmit(CAN1,&tx);
					else
						CAN_ITConfig(CAN1, CAN_IT_TME, DISABLE);						
}
/*******************************************************************************/
void 			CAN2_TX_IRQHandler(void)
{
CanTxMsg	tx;
					if(_buffer_pull(__can->tx,&tx,sizeof(CanTxMsg)))
						CAN_Transmit(CAN2,&tx);
					else
						CAN_ITConfig(CAN2, CAN_IT_TME, DISABLE);						
}
/*******************************************************************************/
void 			CAN1_RX0_IRQHandler(void)
{
CanRxMsg	rx;
					CAN_Receive(CAN1, CAN_FIFO0, &rx);
					_buffer_push(__can->rx,&rx,sizeof(CanRxMsg));
}
/*******************************************************************************/
void 			CAN2_RX0_IRQHandler(void)
{
CanRxMsg	rx;
					CAN_Receive(CAN2, CAN_FIFO0, &rx);
					_buffer_push(__can->rx,&rx,sizeof(CanRxMsg));
}
/*******************************************************************************/
void 			CAN1_SCE_IRQHandler(void) {
					CAN1->MSR |= 0x0004;
					}
/*******************************************************************************/
void 			CAN2_SCE_IRQHandler(void) {
					CAN2->MSR |= 0x0004;
					}
/**
* @}
*/ 

