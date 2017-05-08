#include "type.h"

#include "usb.h"
#include "cdc.h"
#include "usbcfg.h"
#include "usbdesc.h"

#include "msc.h"
#include "config.h"

/* USB Standard Device Descriptor */
__ALIGN_BEGIN uint8_t USBD_VCP_DeviceDesc[] __ALIGN_END = {
		0x12,              /* bLength */
		USB_DEVICE_DESCRIPTOR_TYPE,        /* bDescriptorType */
		WBVAL(0x0200), /* 2.0 */           /* bcdUSB */
		USB_DEVICE_CLASS_MISCELLANEOUS,    /* bDeviceClass */
		0x02,                              /* bDeviceSubClass */
		0x01,                              /* bDeviceProtocol */
		USB_MAX_PACKET0,                   /* bMaxPacketSize0 */
		WBVAL(USB_VENDOR_ID),              /* idVendor */
		WBVAL(USB_PROD_ID),                /* idProduct */
		WBVAL(USB_DEVICE), /* 1.00 */      /* bcdDevice */
		0x01,                              /* iManufacturer */
		0x02,                              /* iProduct */
		0x03,                              /* iSerialNumber */
		0x01                               /* bNumConfigurations: one possible configuration*/
};

/* USB String Descriptor (optional) */
uint8_t USBD_LANGID_STRING[] = {
		/* Index 0x00: LANGID Codes */
		0x04,                              /* bLength */
		USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
		WBVAL(0x0409) /* US English */     /* wLANGID */
};

uint8_t USBD_MANUFACTURER_STRING[] = {
	/* Index 0x01: Manufacturer */
		(13*2 + 2),                        /* bLength (13 Char + Type + lenght) */
		USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
		'N',0,
		'X',0,
		'P',0,
		' ',0,
		'S',0,
		'E',0,
		'M',0,
		'I',0,
		'C',0,
		'O',0,
		'N',0,
		'D',0,
		' ',0
};

uint8_t USBD_PRODUCT_FS_STRING[] = {
		/* Index 0x02: Product */
		(21*2 + 2),                        /* bLength ( 21 Char + Type + lenght) */
		USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
		'N',0,
		'X',0,
		'P',0,
		' ',0,
		'L',0,
		'P',0,
		'C',0,
		'1',0,
		'3',0,
		'x',0,
		'x',0,
		' ',0,
		'M',0,
		'S',0,
		'D',0,
		'/',0,
		'V',0,
		'C',0,
		'O',0,
		'M',0,
		' ',0
	};
	
uint8_t USBD_SERIALNUMBER_FS_STRING[] = {	
		/* Index 0x03: Serial Number */
		(16*2 + 2),                        /* bLength (12 Char + Type + lenght) */
		USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
		'C',0,
		'O',0,
		'M',0,
		'P',0,
		'O',0,
		'S',0,
		'I',0,
		'T',0,
		'E',0,
		' ',0,
		'D',0,
		'E',0,
		'M',0,
		'O',0,
		' ',0,
		' ',0
};

uint8_t USBD_CONFIGURATION_FS_STRING[] = {	
		/* Index 0x03: Serial Number */
		(16*2 + 2),                        /* bLength (12 Char + Type + lenght) */
		USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
		'C',0,
		'O',0,
		'M',0,
		'P',0,
		'O',0,
		'S',0,
		'I',0,
		'T',0,
		'E',0,
		' ',0,
		'C',0,
		'O',0,
		'N',0,
		'F',0,
		'I',0,
		'G',0
};

uint8_t USBD_INTERFACE_FS_STRING[] = {
		/* Index 0x04: Interface 0, Alternate Setting 0 */
		( 6*2 + 2),                        /* bLength (6 Char + Type + lenght) */
		USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
		'M',0,
		'e',0,
		'm',0,
		'o',0,
		'r',0,
		'y',0,
		/* Index 0x05: Interface 0, Alternate Setting 0 */
		( 4*2 + 2),                        /* bLength (4 Char + Type + lenght) */
		USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
		'V',0,
		'C',0,
		'O',0,
		'M',0,

		/* Index 0x05: Interface 0, Alternate Setting 0 */
		( 8*2 + 2),                        /* bLength (4 Char + Type + lenght) */
		USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
		'C',0,
		'O',0,
		'M',0,
		'/',0,
		'D',0,
		'A',0,
		'T',0,
		'A',0,
};
