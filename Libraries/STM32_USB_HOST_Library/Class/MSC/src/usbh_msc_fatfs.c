#include "usb_conf.h"
#include "diskio.h"
#include "usbh_msc_core.h"
#include "usbd_msc_mem.h"
/*--------------------------------------------------------------------------

Module Private Functions and Variables

---------------------------------------------------------------------------*/
static volatile DSTATUS Stat = STA_NOINIT;	/* Disk status */

extern USB_OTG_CORE_HANDLE          USB_OTG_Core;
extern USBH_HOST                    USB_Host;
extern USBD_STORAGE_cb_TypeDef			USBD_MICRO_SDIO_fops;

/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize	(
            BYTE drv		/* Physical drive number (0) */
						)
{
	if(drv==0)	return(USBD_MICRO_SDIO_fops.Init(drv));
  
  if(HCD_IsDeviceConnected(&USB_OTG_Core))
  {  
    Stat &= ~STA_NOINIT;
  }
 
  return Stat;
}



/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
					BYTE drv		/* Physical drive number (0) */
					)
{
	if(drv==0)	return(USBD_MICRO_SDIO_fops.IsReady(drv));
	return Stat;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
                   BYTE drv,			/* Physical drive number (0) */
                   BYTE *buff,		/* Pointer to the data buffer to store read data */
                   DWORD sector,	/* Start sector number (LBA) */
                   UINT count			/* Sector count (1..255) */
                  )
{
  BYTE status = USBH_MSC_OK;  
  if(drv==0)	
		return (DRESULT)USBD_MICRO_SDIO_fops.Read(drv,buff,sector,count);
  if (Stat & STA_NOINIT) return RES_NOTRDY;
  if(HCD_IsDeviceConnected(&USB_OTG_Core))
  {  
    do
    {
      status = USBH_MSC_Read10(&USB_OTG_Core, buff,sector,512 * count);
      USBH_MSC_HandleBOTXfer(&USB_OTG_Core ,&USB_Host);
      
      if(!HCD_IsDeviceConnected(&USB_OTG_Core))
      { 
        return RES_ERROR;
      }      
    }
    while(status == USBH_MSC_BUSY );
  }
  
  if(status == USBH_MSC_OK)
    return RES_OK;
  return RES_ERROR;
  
}
/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/
#if _READONLY == 0
DRESULT disk_write (
                    BYTE drv,					/* Physical drive number (0) */
                    const BYTE *buff,	/* Pointer to the data to be written */
                    DWORD sector,			/* Start sector number (LBA) */
                    UINT count				/* Sector count (1..255) */
                   )
{
  BYTE status = USBH_MSC_OK;
  if(drv==0)	
		return (DRESULT)USBD_MICRO_SDIO_fops.Write(drv,(uint8_t *)buff,sector,count);
  if (Stat & STA_NOINIT) return RES_NOTRDY;
  if (Stat & STA_PROTECT) return RES_WRPRT;
  
  
  if(HCD_IsDeviceConnected(&USB_OTG_Core))
  {  
    do
    {
      status = USBH_MSC_Write10(&USB_OTG_Core,(BYTE*)buff,sector,512 * count);
      USBH_MSC_HandleBOTXfer(&USB_OTG_Core, &USB_Host);
      
      if(!HCD_IsDeviceConnected(&USB_OTG_Core))
      { 
        return RES_ERROR;
      }
    }
    
    while(status == USBH_MSC_BUSY );
    
  }
  
  if(status == USBH_MSC_OK)
    return RES_OK;
  return RES_ERROR;
}
#endif /* _READONLY == 0 */


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

#if _USE_IOCTL == 1
DRESULT disk_ioctl (
                    BYTE drv,		/* Physical drive number (0) */
                    BYTE ctrl,		/* Control code */
                    void *buff		/* Buffer to send/receive control data */
                   )
{
 
  DRESULT res = RES_ERROR;
	
	switch (ctrl) {
  case CTRL_SYNC :		/* Make sure that no pending write process */
    res = RES_OK;
    break;
    
  case GET_SECTOR_COUNT :	/* Get number of sectors on the disk (DWORD) */
		if(drv==0)
			*(DWORD*)buff = (DWORD) SECTOR_COUNT;
		else
			*(DWORD*)buff = (DWORD) USBH_MSC_Param.MSCapacity;
    res = RES_OK;
    break;
    
  case GET_SECTOR_SIZE :	/* Get R/W sector size (WORD) */
		if(drv==0)
			*(DWORD*)buff = (DWORD) SECTOR_SIZE;
		else
			*(WORD*)buff = 512;
    res = RES_OK;
    break;
    
  case GET_BLOCK_SIZE :	/* Get erase block size in unit of sector (DWORD) */
		if(drv==0)
			*(DWORD*)buff = (DWORD) PAGE_SIZE;
		else
			*(DWORD*)buff = 512;
    res = RES_OK;    
    break;
   
  default:
    res = RES_PARERR;
  }
	if(drv==0)
		return res; 
	if (Stat & STA_NOINIT) return RES_NOTRDY;
		return res; 	

}
#endif /* _USE_IOCTL != 0 */
