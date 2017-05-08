#include "type.h"

#include "usb.h"
#include "cdc.h"
#include "usbcfg.h"
#include "usbdesc.h"

#include "msc.h"
#include "config.h"

/* USB Configuration Descriptor */
/*   All Descriptors (Configuration, Interface, Endpoint, Class, Vendor) */

__ALIGN_BEGIN uint8_t usbd_cdc_CfgDesc[] __ALIGN_END = {
		/* Configuration 1 */
		USB_CONFIGUARTION_DESC_SIZE,       /* bLength */
		USB_CONFIGURATION_DESCRIPTOR_TYPE, /* bDescriptorType */
		WBVAL(                             /* wTotalLength */
				1*USB_CONFIGUARTION_DESC_SIZE +
				1*USB_INTERFACE_DESC_SIZE     +  /* mass storage interface */
				2*USB_ENDPOINT_DESC_SIZE      +  /* bulk endpoints */
				1*USB_INTERFACE_ASSOCIATION_DESC_SIZE +  /* interface association */
				1*USB_INTERFACE_DESC_SIZE     +  /* communication interface */
				0x0013                        +  /* CDC functions */
				1*USB_ENDPOINT_DESC_SIZE      +  /* interrupt endpoint */
				1*USB_INTERFACE_DESC_SIZE     +  /* data interface */
				2*USB_ENDPOINT_DESC_SIZE      +  /* bulk endpoints */
				0
		),

		0x03,                              /* bNumInterfaces */
		0x01,                              /* bConfigurationValue: 0x01 is used to select this configuration */
		0x00,                              /* iConfiguration: no string to describe this configuration */
		USB_CONFIG_BUS_POWERED /*|*/       /* bmAttributes */
		/*USB_CONFIG_REMOTE_WAKEUP*/,
		USB_CONFIG_POWER_MA(100),          /* bMaxPower, device power consumption is 100 mA */

		/* Interface 0, Alternate Setting 0, MSC Class */
		USB_INTERFACE_DESC_SIZE,           /* bLength */
		USB_INTERFACE_DESCRIPTOR_TYPE,     /* bDescriptorType */
		USB_MSC_IF_NUM,                    /* bInterfaceNumber */
		0x00,                              /* bAlternateSetting */
		0x02,                              /* bNumEndpoints */
		USB_DEVICE_CLASS_STORAGE,          /* bInterfaceClass */
		MSC_SUBCLASS_SCSI,                 /* bInterfaceSubClass */
		MSC_PROTOCOL_BULK_ONLY,            /* bInterfaceProtocol */
		0x04,                              /* iInterface */

		/* Endpoint, EP2 Bulk IN */
		USB_ENDPOINT_DESC_SIZE,            /* bLength */
		USB_ENDPOINT_DESCRIPTOR_TYPE,      /* bDescriptorType */
		USB_ENDPOINT_IN(2),                /* bEndpointAddress */
		USB_ENDPOINT_TYPE_BULK,            /* bmAttributes */
		WBVAL(0x0040),                     /* wMaxPacketSize */
		0x00,                              /* bInterval: ignore for Bulk transfer */

		/* Endpoint, EP2 Bulk OUT */
		USB_ENDPOINT_DESC_SIZE,            /* bLength */
		USB_ENDPOINT_DESCRIPTOR_TYPE,      /* bDescriptorType */
		USB_ENDPOINT_OUT(2),               /* bEndpointAddress */
		USB_ENDPOINT_TYPE_BULK,            /* bmAttributes */
		WBVAL(0x0040),                     /* wMaxPacketSize */
		0x00,                              /* bInterval: ignore for Bulk transfer */

		/* IAD to associate the two CDC interfaces */
		USB_INTERFACE_ASSOCIATION_DESC_SIZE,       /* bLength */
		USB_INTERFACE_ASSOCIATION_DESCRIPTOR_TYPE, /* bDescriptorType */
		USB_CDC_CIF_NUM,                   /* bFirstInterface */
		2,                                 /* bInterfaceCount */
		CDC_COMMUNICATION_INTERFACE_CLASS, /* bFunctionClass */
		CDC_ABSTRACT_CONTROL_MODEL,        /* bFunctionSubClass */
		0,                                 /* bFunctionProtocol */
		0x06,                              /* iFunction (Index of string descriptor describing this function) */

		/* Interface 0, Alternate Setting 0, Communication class interface descriptor */
		USB_INTERFACE_DESC_SIZE,           /* bLength */
		USB_INTERFACE_DESCRIPTOR_TYPE,     /* bDescriptorType */
		USB_CDC_CIF_NUM,                   /* bInterfaceNumber: Number of Interface */
		0x00,                              /* bAlternateSetting: Alternate setting */
		0x01,                              /* bNumEndpoints: One endpoint used */
		CDC_COMMUNICATION_INTERFACE_CLASS, /* bInterfaceClass: Communication Interface Class */
		CDC_ABSTRACT_CONTROL_MODEL,        /* bInterfaceSubClass: Abstract Control Model */
		0x00,                              /* bInterfaceProtocol: no protocol used */
		0x05,                              /* iInterface: */
		/*Header Functional Descriptor*/
		0x05,                              /* bLength: Endpoint Descriptor size */
		CDC_CS_INTERFACE,                  /* bDescriptorType: CS_INTERFACE */
		CDC_HEADER,                        /* bDescriptorSubtype: Header Func Desc */
		WBVAL(CDC_V1_10), /* 1.10 */       /* bcdCDC */
		/*Call Management Functional Descriptor*/
		0x05,                              /* bFunctionLength */
		CDC_CS_INTERFACE,                  /* bDescriptorType: CS_INTERFACE */
		CDC_CALL_MANAGEMENT,               /* bDescriptorSubtype: Call Management Func Desc */
		0x01,                              /* bmCapabilities: device handles call management */
		USB_CDC_DIF_NUM,               	   /* bDataInterface: CDC data IF ID */
		/*Abstract Control Management Functional Descriptor*/
		0x04,                              /* bFunctionLength */
		CDC_CS_INTERFACE,                  /* bDescriptorType: CS_INTERFACE */
		CDC_ABSTRACT_CONTROL_MANAGEMENT,   /* bDescriptorSubtype: Abstract Control Management desc */
		0x02,                              /* bmCapabilities: SET_LINE_CODING, GET_LINE_CODING, SET_CONTROL_LINE_STATE supported */
		/*Union Functional Descriptor*/
		0x05,                              /* bFunctionLength */
		CDC_CS_INTERFACE,                  /* bDescriptorType: CS_INTERFACE */
		CDC_UNION,                         /* bDescriptorSubtype: Union func desc */
		USB_CDC_CIF_NUM,                   /* bMasterInterface: Communication class interface is master */
		USB_CDC_DIF_NUM,                   /* bSlaveInterface0: Data class interface is slave 0 */
		/*Endpoint 1 Descriptor*/            /* event notification (optional) */
		USB_ENDPOINT_DESC_SIZE,            /* bLength */
		USB_ENDPOINT_DESCRIPTOR_TYPE,      /* bDescriptorType */
		USB_ENDPOINT_IN(1),                /* bEndpointAddress */
		USB_ENDPOINT_TYPE_INTERRUPT,       /* bmAttributes */
		WBVAL(0x0010),                     /* wMaxPacketSize */
		0x02,          /* 2ms */           /* bInterval */
		/* Interface 1, Alternate Setting 0, Data class interface descriptor*/
		USB_INTERFACE_DESC_SIZE,           /* bLength */
		USB_INTERFACE_DESCRIPTOR_TYPE,     /* bDescriptorType */
		USB_CDC_DIF_NUM,                   /* bInterfaceNumber: Number of Interface */
		0x00,                              /* bAlternateSetting: no alternate setting */
		0x02,                              /* bNumEndpoints: two endpoints used */
		CDC_DATA_INTERFACE_CLASS,          /* bInterfaceClass: Data Interface Class */
		0x00,                              /* bInterfaceSubClass: no subclass available */
		0x00,                              /* bInterfaceProtocol: no protocol used */
		0x05,                              /* iInterface: */
		/* Endpoint, EP3 Bulk Out */
		USB_ENDPOINT_DESC_SIZE,            /* bLength */
		USB_ENDPOINT_DESCRIPTOR_TYPE,      /* bDescriptorType */
		USB_ENDPOINT_OUT(3),               /* bEndpointAddress */
		USB_ENDPOINT_TYPE_BULK,            /* bmAttributes */
		WBVAL(USB_CDC_BUFSIZE),            /* wMaxPacketSize */
		0x00,                              /* bInterval: ignore for Bulk transfer */
		/* Endpoint, EP3 Bulk In */
		USB_ENDPOINT_DESC_SIZE,            /* bLength */
		USB_ENDPOINT_DESCRIPTOR_TYPE,      /* bDescriptorType */
		USB_ENDPOINT_IN(3),                /* bEndpointAddress */
		USB_ENDPOINT_TYPE_BULK,            /* bmAttributes */
		WBVAL(USB_CDC_BUFSIZE),            /* wMaxPacketSize */
		0x00,                              /* bInterval: ignore for Bulk transfer */

		/* Terminator */
		0                                  /* bLength */
};

extern USBD_Class_cb_TypeDef  USBD_CDC_cb;
extern USBD_Class_cb_TypeDef  USBD_MSC_cb;

uint8_t  usbd_x_Init(void *pdev , uint8_t cfgidx)
{
	return	USBD_CDC_cb.Init(pdev,  cfgidx);
}

uint8_t  usbd_x_DeInit(void *pdev , uint8_t cfgidx)
{
	return USBD_CDC_cb.DeInit(pdev,  cfgidx);	
}

uint8_t  usbd_x_Setup(void *pdev , USB_SETUP_REQ  *req)
{
	return	USBD_CDC_cb.Setup(pdev,  req);
}

uint8_t  usbd_x_EP0_RxReady(void *pdev)
{
	return	USBD_CDC_cb.EP0_RxReady(pdev);
}

uint8_t  usbd_x_SOF(void *pdev)
{
	return	USBD_CDC_cb.SOF(pdev);
}

uint8_t  usbd_x_DataIn(void *pdev , uint8_t epnum)
{
	return	USBD_CDC_cb.DataIn(pdev,  epnum);
}

uint8_t  usbd_x_DataOut (void *pdev , uint8_t epnum)
{
	return	USBD_CDC_cb.DataOut(pdev,  epnum);
}

uint8_t  *USBD_x_GetCfgDesc(uint8_t speed , uint16_t *length)
{
	return	USBD_CDC_cb.GetConfigDescriptor(speed , length);
}

USBD_Class_cb_TypeDef  USBD_CDC_cb = 
{
  usbd_x_Init,
  usbd_x_DeInit,
  usbd_x_Setup,
  NULL,
  usbd_x_EP0_RxReady,
  usbd_x_DataIn,
  usbd_x_DataOut,
  usbd_x_SOF,
  NULL,
  NULL,     
  USBD_x_GetCfgDesc
};

/*
  uint8_t  (*Init)         (void *pdev , uint8_t cfgidx);
  uint8_t  (*DeInit)       (void *pdev , uint8_t cfgidx);

  uint8_t  (*Setup)        (void *pdev , USB_SETUP_REQ  *req);  
  uint8_t  (*EP0_TxSent)   (void *pdev );    
  uint8_t  (*EP0_RxReady)  (void *pdev );  

  uint8_t  (*DataIn)       (void *pdev , uint8_t epnum);   
  uint8_t  (*DataOut)      (void *pdev , uint8_t epnum); 
  uint8_t  (*SOF)          (void *pdev); 
  uint8_t  (*IsoINIncomplete)  (void *pdev); 
  uint8_t  (*IsoOUTIncomplete)  (void *pdev);   

  uint8_t  *(*GetConfigDescriptor)( uint8_t speed , uint16_t *length); 
#ifdef USB_OTG_HS_CORE 
  uint8_t  *(*GetOtherConfigDescriptor)( uint8_t speed , uint16_t *length);   
#endif

#ifdef USB_SUPPORT_USER_STRING_DESC 
  uint8_t  *(*GetUsrStrDescriptor)( uint8_t speed ,uint8_t index,  uint16_t *length);   
#endif  
  
} USBD_Class_cb_TypeDef;
*/
