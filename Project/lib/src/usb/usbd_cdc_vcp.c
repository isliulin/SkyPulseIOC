#include "app.h"
/**
  ******************************************************************************
  * @file    usbd_cdc_if_template.c
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    19-March-2012
  * @brief   Generic media access Layer.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */ 

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED 
#pragma     data_alignment = 4 
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */

/* Includes ------------------------------------------------------------------*/
#include "usbd_cdc_if_template.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static uint16_t VCP_Init     (void);
static uint16_t VCP_DeInit   (void);
static uint16_t VCP_Ctrl     (uint32_t Cmd, uint8_t* Buf, uint32_t Len);
static uint16_t VCP_DataTx   (uint8_t* Buf, uint32_t Len);
static uint16_t VCP_DataRx   (uint8_t* Buf, uint32_t Len);

CDC_IF_Prop_TypeDef VCP_fops = 
{
  VCP_Init,
  VCP_DeInit,
  VCP_Ctrl,
  VCP_DataTx,
  VCP_DataRx
};
//_____________________________________________________________________________________
/**
  * @brief  VCP_DataTx
  *         CDC received data to be send over USB IN endpoint are managed in 
  *         this function.
  * @param  Buf: Buffer of data to be sent
  * @param  Len: Number of data to be sent (in bytes)
  * @retval Result of the opeartion: USBD_OK if all operations are OK else VCP_FAIL
  */
//_____________________________________________________________________________________
extern 		uint8_t 	APP_Rx_Buffer [];
extern 		uint32_t	APP_Rx_ptr_in;
extern 		uint32_t	APP_Rx_ptr_out;
static 		_io				*__com;
//_____________________________________________________________________________________	
uint16_t	VCP_DataTx (uint8_t* Buf, uint32_t Len) {
uint32_t	i=APP_Rx_ptr_in;
					while(Len--) {
						APP_Rx_Buffer[i++] = *Buf++;
						i%=APP_RX_DATA_SIZE;
						if(i==APP_Rx_ptr_out%APP_RX_DATA_SIZE)
							return USBD_BUSY;
					}
					APP_Rx_ptr_in=i;
					return USBD_OK;
}
//_____________________________________________________________________________________
/**
  * @brief  VCP_DataRx
  *         Data received over USB OUT endpoint are sent over CDC interface 
  *         through this function.
  *           
  *         @note
  *         This function will block any OUT packet reception on USB endpoint 
  *         untill exiting this function. If you exit this function before transfer
  *         is complete on CDC interface (ie. using DMA controller) it will result 
  *         in receiving more data while previous ones are still not sent.
  *                 
  * @param  Buf: Buffer of data to be received
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the opeartion: USBD_OK if all operations are OK else VCP_FAIL
  */
uint16_t	VCP_DataRx (uint8_t* Buf, uint32_t Len)
{
					if(__com)
						_buffer_push(__com->rx,Buf,Len);
					return USBD_OK;
}
//_____________________________________________________________________________________
int				putVCP (_buffer *p, int	c) {
					if(VCP_DataTx((uint8_t *)&c,1)==USBD_OK)
						return(c);
					else
						return(EOF);
}
//_____________________________________________________________________________________
int				getVCP (_buffer *p) {
int				i=0;
					if(_buffer_pull(p,&i,1))
						return i;
					else
						return EOF;
}
//_____________________________________________________________________________________
/* Private functions
*/
/**
  * @brief  VCP_Init
  *         Initializes the CDC media low layer
  * @param  None
  * @retval Result of the opeartion: USBD_OK if all operations are OK else USBD_FAIL
  */
static uint16_t VCP_Init(void)
{
					__com=_io_init(128,128);
					__com->get= getVCP;	
					__com->put= putVCP;

					_thread_add(ParseCom,__com,"ParseCom USB",0);
					return USBD_OK;
}
//_____________________________________________________________________________________
/**
  * @brief  VCP_DeInit
  *         DeInitializes the CDC media low layer
  * @param  None
  * @retval Result of the opeartion: USBD_OK if all operations are OK else USBD_FAIL
  */
static uint16_t VCP_DeInit(void)
{
					_thread_remove(ParseCom,__com);
					_io_close(__com);
					return USBD_OK;
}
//_____________________________________________________________________________________
struct LINE_CODING 
  {
    uint32_t bitrate;
    uint8_t format;
    uint8_t paritytype;
    uint8_t datatype;
  } linecoding =	{115200,0x00,0x00,0x08};

/**
  * @brief  VCP_Ctrl
  *         Manage the CDC class requests
  * @param  Cmd: Command code            
  * @param  Buf: Buffer containing command data (request parameters)
  * @param  Len: Number of data to be sent (in bytes)
  * @retval Result of the opeartion: USBD_OK if all operations are OK else USBD_FAIL
  */
static uint16_t VCP_Ctrl (uint32_t Cmd, uint8_t* Buf, uint32_t Len)
{
  switch (Cmd)
  {
  case SET_LINE_CODING:
    linecoding.bitrate = (uint32_t)(Buf[0] | (Buf[1] << 8) | (Buf[2] << 16) | (Buf[3] << 24));
    linecoding.format = Buf[4];
    linecoding.paritytype = Buf[5];
    linecoding.datatype = Buf[6];
		break;

  case GET_LINE_CODING:
    Buf[0] = (uint8_t)(linecoding.bitrate);
    Buf[1] = (uint8_t)(linecoding.bitrate >> 8);
    Buf[2] = (uint8_t)(linecoding.bitrate >> 16);
    Buf[3] = (uint8_t)(linecoding.bitrate >> 24);
    Buf[4] = linecoding.format;
    Buf[5] = linecoding.paritytype;
    Buf[6] = linecoding.datatype; 
    break;

  case SEND_ENCAPSULATED_COMMAND:
  case GET_ENCAPSULATED_RESPONSE:
  case SET_COMM_FEATURE:
  case GET_COMM_FEATURE:
  case CLEAR_COMM_FEATURE:
	case SET_CONTROL_LINE_STATE:
	case SEND_BREAK:   
  default:
    break;
  }
  return USBD_OK;
}

