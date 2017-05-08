/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2007        */
/*-----------------------------------------------------------------------*/
/* This is a stub disk I/O module that acts as front end of the existing */
/* disk I/O modules and attach it to FatFs module with common interface. */
/*-----------------------------------------------------------------------*/
#include 				<string.h>
#include 				<stdio.h>
#include 				<time.h>

#include				"iap.h"
#include 				"diskio.h"
/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
DRESULT disk_ioctl (
	BYTE drv,			/* Physical drive nmuber (0..) */
	BYTE ctrl,			/* Control code */
	void *buff			/* Buffer to send/receive control data */
)
{
	switch(ctrl) {
		case CTRL_SYNC:
			break;
		case GET_SECTOR_SIZE:
			*(int *)buff=SECTOR_SIZE;
			break;
		case GET_SECTOR_COUNT:
			*(int *)buff=SECTOR_COUNT;
			break;
		case GET_BLOCK_SIZE:
			*(int *)buff=ERASE_SIZE;
			break;
		case CTRL_ERASE_SECTOR:
#if _USE_ERASE
%error "define how to erase block... ???"
#endif
			break;
		case CTRL_POWER:
		case CTRL_LOCK:
		case CTRL_EJECT:
		case MMC_GET_TYPE:
		case MMC_GET_CSD:
		case MMC_GET_CID:
		case MMC_GET_OCR:
		case MMC_GET_SDSTAT:
		case ATA_GET_REV:
		case ATA_GET_MODEL:
		case ATA_GET_SN:
			break;
	}
  	return RES_OK; 
}
/*-----------------------------------------------------------------------*/
/* Return Disk Status                                                    */

DSTATUS disk_status (BYTE drv) 
{
  return RES_OK; 
}
/*-----------------------------------------------------------------------*/
/* Inicializes a Drive                                                   */
#ifndef WIN32
DSTATUS disk_initialize (BYTE drv)
{
	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
	return RES_OK; 
}
/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
//
//
DRESULT disk_read (
	BYTE drv,			/* Physical drive nmuber (0..) */
	BYTE *buff,			/* Data buffer to store read data */
	DWORD sector,		/* Sector address (LBA) */
	BYTE count			/* Number of sectors to read (1..255) */
)
{
int i,*p,*q=NULL;
	for(p=(int *)STORAGE_TOP; p[SECTOR_SIZE/4]!=-1; p=&p[SECTOR_SIZE/4+1])
		if(p[SECTOR_SIZE/4] == sector)
			q=p;
	if(q)
		p=q;
	q=(int *)buff;
	for(i=0;i<SECTOR_SIZE/4; ++i)
		*q++=~(*p++);	
	if(--count)
		disk_read (drv, (uint8_t *)q, ++sector, count);
	return RES_OK; 
}
/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
//
//
#if _READONLY == 0
DRESULT disk_write (
	BYTE drv,			/* Physical drive nmuber (0..) */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address (LBA) */
	BYTE count			/* Number of sectors to write (1..255) */
)
{
int i,*p,*q=NULL;
	for(p=(int *)STORAGE_TOP; p[SECTOR_SIZE/4]!=-1; p=&p[SECTOR_SIZE/4+1])
		if(p[SECTOR_SIZE/4] == sector)
			q=p;
		
	q=(int *)buff;
	for(i=0; i<SECTOR_SIZE/4; ++i)	
		if(*q++)
			break;

	if(i<SECTOR_SIZE/4) {												// all zeroes ???
		q=(int *)buff;
		for(i=0; i<SECTOR_SIZE/4; ++i,++p,++q)
			FlashProgram32((int)p,~(*q));
		FlashProgram32((int)p,sector);
	}
	if(--count)
		disk_write (drv, (uint8_t *)q, ++sector, count);
	return RES_OK; 
}	  
#endif /* _READONLY */
/*-----------------------------------------------------------------------*/
void SectorQuery(void) {
int i,j,*p,*q;

	p=(int *)STORAGE_TOP;
	for(i=0; i<SECTOR_COUNT; ++i) {
		if(!((i%255)%16))
			printf("\r\n");
		if(!(i%255))
			printf("\r\n");		
		if(p[SECTOR_SIZE/4] == -1)
			printf(" ---");
		else {
			q=&p[SECTOR_SIZE/4+1];
			j=i;
			while(++j<SECTOR_COUNT && p[SECTOR_SIZE/4] != q[SECTOR_SIZE/4])
				q=&q[SECTOR_SIZE/4+1];
			if(j==SECTOR_COUNT)
				printf(" %-3d",p[SECTOR_SIZE/4]);
			else
				printf("%c%-3d",'*',p[SECTOR_SIZE/4]);
		}
		p=&p[SECTOR_SIZE/4+1];
	}
}
/*-----------------------------------------------------------------------*/
void Defragment(BYTE drv) {

int i,f,e,*p,*q,buf[SECTOR_SIZE/4];
	
	f=FLASH_Sector_5;
	e=ERASE_SIZE;
	p=(int *)STORAGE_TOP;

	do {
		do {
			q=&p[SECTOR_SIZE/4+1];
			while(p[SECTOR_SIZE/4] != q[SECTOR_SIZE/4] && q[SECTOR_SIZE/4] != -1)
				q=&q[SECTOR_SIZE/4+1];
			if(q[SECTOR_SIZE/4] == -1) {
				for(i=0; i<SECTOR_SIZE/4;++i)
					buf[i]=~p[i];
				Watchdog();
				disk_write (drv,(BYTE *)buf,p[SECTOR_SIZE/4],1);
			}
			p=&p[SECTOR_SIZE/4+1]; 
		} while(((int)p)-STORAGE_TOP <  e && p[SECTOR_SIZE/4] != -1);
		FLASH_EraseSector(f, VoltageRange_3);
		printf(".");
		f+=8; 
		e+=ERASE_SIZE;
	} while(p[SECTOR_SIZE/4] != -1);	
	FLASH_EraseSector(f, VoltageRange_3);
}
#else

char	disk[SECTOR_SIZE*SECTOR_COUNT];
DSTATUS disk_initialize (BYTE drv)			/* Physical drive nmuber(0..)*/
{
	return RES_OK; 
}
/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
DRESULT disk_read (
	BYTE drv,			/* Physical drive nmuber (0..) */
	BYTE *buff,			/* Data buffer to store read data */
	DWORD sector,		/* Sector address (LBA) */
	BYTE count			/* Number of sectors to read (1..255) */
)
{
	char s[32];
	FILE *f;

	memset(buff,0,SECTOR_SIZE);	
	sprintf(s,"S%d%cdat",sector,'.');

	f=fopen(s,"rb");
	if(f)
		fread(buff,SECTOR_SIZE,1,f);
	if(--count)
		disk_read(drv,&buff[SECTOR_SIZE],++sector,--count);
  	return RES_OK; 
}
/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */

#if _READONLY == 0
DRESULT disk_write (
	BYTE drv,						/* Physical drive nmuber (0..) */
	const BYTE *buff,		/* Data to be written */
	DWORD sector,				/* Sector address (LBA) */
	BYTE count					/* Number of sectors to write (1..255) */
)
{
	char s[32];
	FILE *f;

	sprintf(s,"S%d%cdat",sector,'.');
	f=fopen(s,"wb");
	if(f)
	{
		fwrite(buff,SECTOR_SIZE,1,f);
		fclose(f);
	}
	if(--count)
		disk_write(drv,&buff[SECTOR_SIZE],++sector,--count);
  	return RES_OK; 
}
#endif /* _READONLY */
#endif
/*-----------------------------------------------------------------------*/
DWORD get_fattime (void)
{
	return 0;
}

#ifndef WIN32
time_t time(time_t *t) {
	return *t;
}
#endif












