
#include "usb_conf.h"
#include "diskio.h"
#include "usbh_msc_core.h"

#include "pfm.h"
/*--------------------------------------------------------------------------

Module Private Functions and Variables

---------------------------------------------------------------------------*/
static volatile DSTATUS Stat = STA_NOINIT;	/* Disk status */

extern USB_OTG_CORE_HANDLE          USB_OTG_Core;
extern USBH_HOST                    USB_Host;

/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize	(
                        BYTE drv		/* Physical drive number (0) */
						)
{
	if(drv==FSDRIVE_CPU)	return(STORAGE_Init(drv));
  
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
	if(drv==FSDRIVE_CPU)	return(STORAGE_IsReady(drv));
	return Stat;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
                   BYTE drv,			/* Physical drive number (0) */
                   BYTE *buff,			/* Pointer to the data buffer to store read data */
                   DWORD sector,		/* Start sector number (LBA) */
                   BYTE count			/* Sector count (1..255) */
                  )
{
  BYTE status = USBH_MSC_OK;  
  if(drv==FSDRIVE_CPU)	return (DRESULT)STORAGE_Read(drv,buff,sector,count);
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
                    BYTE drv,			/* Physical drive number (0) */
                    const BYTE *buff,	/* Pointer to the data to be written */
                    DWORD sector,		/* Start sector number (LBA) */
                    BYTE count			/* Sector count (1..255) */
                   )
{
  BYTE status = USBH_MSC_OK;
  if(drv==FSDRIVE_CPU)	return (DRESULT)STORAGE_Write(drv,(uint8_t *)buff,sector,count);
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

#if _USE_IOCTL != 0
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
		if(drv==FSDRIVE_CPU)
			*(DWORD*)buff = (DWORD) SECTOR_COUNT;
		else
			*(DWORD*)buff = (DWORD) USBH_MSC_Param.MSCapacity;
    res = RES_OK;
    break;
    
  case GET_SECTOR_SIZE :	/* Get R/W sector size (WORD) */
		if(drv==FSDRIVE_CPU)
			*(DWORD*)buff = (DWORD) SECTOR_SIZE;
		else
			*(WORD*)buff = 512;
    res = RES_OK;
    break;
    
  case GET_BLOCK_SIZE :	/* Get erase block size in unit of sector (DWORD) */
    if(drv==FSDRIVE_CPU)
			*(DWORD*)buff = (DWORD) ERASE_SIZE;
		else
			*(DWORD*)buff = 512;
    res = RES_OK;    
    break;
   
  default:
    res = RES_PARERR;
  }
	if(drv==FSDRIVE_CPU)
		return res; 
	if (Stat & STA_NOINIT) return RES_NOTRDY;
		return res; 	

}
#endif /* _USE_IOCTL != 0 */
