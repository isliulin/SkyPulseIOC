/**
  ******************************************************************************
  * @file    USBD_usr.c
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    19-March-2012
  * @brief   This file includes the user application layer
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

/* Includes ------------------------------------------------------------------*/
#include "usbd_usr.h"
#include "usbd_ioreq.h"
#include "usb_conf.h"
#include "stdio.h"
#include "app.h"
/** @addtogroup USBD_USER
* @{
*/

#ifdef __GNUC__
/* With GCC/RAISONANCE, small LCD_UsrLog (option LD Linker->Libraries->Small LCD_UsrLog
set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

/** @addtogroup USBD_MSC_DEMO_USER_cb
* @{
*/

/** @defgroup USBD_USR 
* @brief    This file includes the user application layer
* @{
*/ 

/** @defgroup USBD_MSC_Private_TypesDefinitions
* @{
*/ 
/**
* @}
*/ 


/** @defgroup USBD_MSC_Private_Defines
* @{
*/ 

/**
* @}
*/ 


/** @defgroup USBD_MSC_Private_Macros
* @{
*/ 
/**
* @}
*/ 


/** @defgroup USBD_MSC_Private_Variables
* @{
*/ 
/*  Points to the DEVICE_PROP structure of current device */
/*  The purpose of this register is to speed up the execution */

USBD_Usr_cb_TypeDef USR_MSC_cb =
{
  USBD_MSC_DeviceInit,
  USBD_MSC_DeviceReset,
  USBD_MSC_DeviceConfigured,
  USBD_MSC_DeviceSuspended,
  USBD_MSC_DeviceResumed,
  
  USBD_MSC_DeviceConnected,
  USBD_MSC_DeviceDisconnected,    
};

USBD_Usr_cb_TypeDef USR_CDC_cb =
{
  USBD_VCP_DeviceInit,
  USBD_VCP_DeviceReset,
  USBD_VCP_DeviceConfigured,
  USBD_VCP_DeviceSuspended,
  USBD_VCP_DeviceResumed,
  
  USBD_VCP_DeviceConnected,
  USBD_VCP_DeviceDisconnected,    
};

/* wrapperji za inicializacije iz com.c, da se izognes includom
*/
__ALIGN_BEGIN
USB_OTG_CORE_HANDLE							USB_OTG_Core;
__ALIGN_END
extern 	USBD_DEVICE 						USR_MSC_desc,
																USR_VCP_desc;
extern 	USBD_Class_cb_TypeDef  	USBD_MSC_cb,
																USBD_CDC_cb;

void	 	Initialize_device_msc(void) {
				_thread_remove(USBHost,NULL);
				USBD_Init(&USB_OTG_Core, USB_OTG_FS_CORE_ID, &USR_MSC_desc, &USBD_MSC_cb, &USR_MSC_cb);
}

void		Initialize_device_vcp(void) {
				_thread_remove(USBHost,NULL);
				USBD_Init(&USB_OTG_Core, USB_OTG_FS_CORE_ID, &USR_VCP_desc, &USBD_CDC_cb, &USR_CDC_cb);
}

/**
* @}
*/

/** @defgroup USBD_MSC_Private_Constants
* @{
*/ 

/**
* @}
*/

/** @defgroup USBD_MSC_Private_FunctionPrototypes
* @{
*/ 
/**
* @}
*/ 


/** @defgroup USBD_MSC_Private_Functions
* @{
*/ 

#define USER_INFORMATION1 "MSC running on High speed."
#define USER_INFORMATION2 "HID running on Full speed."

/**
* @brief  USBD_MSC_Init 
*         Displays the message on LCD for host lib initialization
* @param  None
* @retval None
*/
void USBD_MSC_DeviceInit(void)
{
#ifdef __PRINTDBG__
	printf("..MSC initialized\r\n");
#endif
}

/**
* @brief  USBD_MSC_DeviceReset 
*         Displays the message on LCD on device attached
* @param  None
* @retval None
*/
void USBD_MSC_DeviceReset(uint8_t speed)
{
#ifdef __PRINTDBG__
	printf("..MSC reset\r\n");
#endif
}


/**
* @brief  USBD_DisconnectEvent
*         Device disconnect event
* @param  None
* @retval Staus
*/
void USBD_MSC_DeviceConfigured (void)
{
#ifdef __PRINTDBG__
	printf("..MSC configured\r\n");
#endif
}
/**
* @brief  USBD_MSC_ResetUSBDevice 
* @param  None
* @retval None
*/
void USBD_MSC_DeviceSuspended(void)
{
#ifdef __PRINTDBG__
	printf("..MSC suspended\r\n");
#endif
}


/**
* @brief  USBD_MSC_ResetUSBDevice 
* @param  None
* @retval None
*/
void USBD_MSC_DeviceResumed(void)
{
	#ifdef __PRINTDBG__
	printf("..MSC resumed\r\n");
#endif
}

/**
* @brief  USBD_USR_DeviceConnected
*         Displays the message on LCD on device connection Event
* @param  None
* @retval Staus
*/
void USBD_MSC_DeviceConnected (void)
{
#ifdef __PRINTDBG__
	printf("..MSC connected\r\n");
#endif
}


/**
* @brief  USBD_USR_DeviceDisonnected
*         Displays the message on LCD on device disconnection Event
* @param  None
* @retval Staus
*/
void USBD_MSC_DeviceDisconnected (void)
{
#ifdef __PRINTDBG__
	printf("..MSC disconnected\r\n");
#endif
}
/*****************************************************************************/
/**
* @brief  USBD_VCP_Init 
*         Displays the message on LCD for host lib initialization
* @param  None
* @retval None
*/
void USBD_VCP_DeviceInit(void)
{
#ifdef __PRINTDBG__
	printf("..VCP initialized\r\n");
#endif
}

/**
* @brief  USBD_MSC_DeviceReset 
*         Displays the message on LCD on device attached
* @param  None
* @retval None
*/
void USBD_VCP_DeviceReset(uint8_t speed)
{
#ifdef __PRINTDBG__
	printf("..VCP reset\r\n");
#endif	
}


/**
* @brief  USBD_DisconnectEvent
*         Device disconnect event
* @param  None
* @retval Staus
*/
void USBD_VCP_DeviceConfigured (void)
{
#ifdef __PRINTDBG__
printf("..VCP configured\r\n");
#endif
}
/**
* @brief  USBD_MSC_ResetUSBDevice 
* @param  None
* @retval None
*/
void USBD_VCP_DeviceSuspended(void)
{
#ifdef __PRINTDBG__
	printf("..VCP suspended\r\n");
#endif
}


/**
* @brief  USBD_MSC_ResetUSBDevice 
* @param  None
* @retval None
*/
void USBD_VCP_DeviceResumed(void)
{
#ifdef __PRINTDBG__
	printf("..VCP resumed\r\n");
#endif
}

/**
* @brief  USBD_USR_DeviceConnected
*         Displays the message on LCD on device connection Event
* @param  None
* @retval Status
*/
void USBD_VCP_DeviceConnected (void)
{
#ifdef __PRINTDBG__
	printf("..VCP connected\r\n");
#endif
}


/**
* @brief  USBD_USR_DeviceDisonnected
*         Displays the message on LCD on device disconnection Event
* @param  None
* @retval Staus
*/
void USBD_VCP_DeviceDisconnected (void)
{
#ifdef __PRINTDBG__
	printf("..VCP disconnected\r\n");
#endif
}
/**
* @}
*/ 

/**
* @}
*/ 

/**
* @}
*/

/**
* @}
*/

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

