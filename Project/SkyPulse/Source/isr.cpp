#include	"stm32f2xx.h"
#include	"isr.h"
extern		"C"  {	
void      SysTick_Handler(void) {
          ++__time__;
          _led(-1,-1);
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
**
*****************************************************************************/
extern 
uint32_t	USB_OTG_Core;
uint32_t	USBH_OTG_ISR_Handler(void *);
uint32_t	USBD_OTG_ISR_Handler(void *);
uint8_t		USB_OTG_IsHostMode(void *);
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void 	OTG_FS_IRQHandler(void) {
					if(USB_OTG_IsHostMode(&USB_OTG_Core))
						USBH_OTG_ISR_Handler(&USB_OTG_Core);
					else
						USBD_OTG_ISR_Handler(&USB_OTG_Core);
					}
}

