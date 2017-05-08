/**
  ******************************************************************************
  * @file    netconf.h
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    07-October-2011 
  * @brief   This file contains all the functions prototypes for the netconf.c 
  *          file.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __NETCONF_H
#define __NETCONF_H

#ifdef __cplusplus
 extern "C" {
#endif
   
/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void LwIP_Init(void);
void LwIP_DHCP_task(void * pvParameters);


// myDef
typedef int int32_t;
typedef short int int16_t;
typedef signed char int8_t;

typedef unsigned int uint32_t;
typedef unsigned short int uint16_t;
typedef unsigned char uint8_t;

#define __IO volatile

void LwIP_Periodic_Handle(__IO uint32_t);	
void LwIP_Pkt_Handle(void);

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

//#define USE_LCD         /* enable LCD  */  
// #define USE_DHCP      /* enable DHCP, if disabled static address is used */
   
/* Uncomment SERIAL_DEBUG to enables retarget of printf to serial port (COM1 on STM32 evalboard) 
   for debug purpose */   
#define SERIAL_DEBUG 

/* MAC ADDRESS: MAC_ADDR0:MAC_ADDR1:MAC_ADDR2:MAC_ADDR3:MAC_ADDR4:MAC_ADDR5 */
#define MAC_ADDR0			0x04
#define MAC_ADDR1			0x01
#define MAC_ADDR2			0x02
#define MAC_ADDR3 		0x01
#define MAC_ADDR4 		0x08
#define MAC_ADDR5 		0x05
 
/*Static IP ADDRESS: IP_ADDR0.IP_ADDR1.IP_ADDR2.IP_ADDR3 */
#define IP_ADDR0   		172
#define IP_ADDR1   		21
#define IP_ADDR2   		244
#define IP_ADDR3   		66
   
/*NETMASK*/
#define NETMASK_ADDR0 255
#define NETMASK_ADDR1 255
#define NETMASK_ADDR2 240
#define NETMASK_ADDR3 0

/*Gateway Address*/
#define GW_ADDR0 		172
#define GW_ADDR1 		21
#define GW_ADDR2 		240
#define GW_ADDR3 		1  

/* Network interface name */
#define IFNAME0 's'
#define IFNAME1 't'

int			putVCP (int	);
int			getVCP (void);
#define 	SLIP_END     		0300 /* 0xC0 */
#define 	SLIP_ESC     		0333 /* 0xDB */
#define 	SLIP_ESC_END 		0334 /* 0xDC */
#define 	SLIP_ESC_ESC 		0335 /* 0xDD */

#ifdef __cplusplus
}
#endif

#endif /* __NETCONF_H */


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
