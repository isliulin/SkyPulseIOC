#include	"pfm.h"
#include	"limits.h"
/*******************************************************************************
* Function Name  : HexChecksumError
* Description    : preverja konsistentnost vrstice iz hex fila
* Input          : pointer na string 
* Output         : 
* Return         : 0 ce je OK
*******************************************************************************/
//	:020000040800F2 
//	:1000000000100020470100085501000857010008B2
//	:10001000590100085B0100085D01000800000000B4
//            .
//            .
//            .
int					HexChecksumError(char *p) {

int	 				err,n=str2hex(&p,2);			
						for(err=n;n-- > -5;err+=str2hex(&p,2));
						return(err & 0xff);
}
/*******************************************************************************
* Function Name  : EraseFLASH
* Description    : Brisanje flash bloka
* Input          :
* Output         :
* Return         :
*******************************************************************************/
int					FlashErase(int n) {
int					i;
						if(n == _BOOT_SECTOR) 
							return(-1);
						FLASH_Unlock();
#if defined (STM32F10X_HD)
						FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);						
						do i=FLASH_ErasePage(n); while(i==FLASH_BUSY);
#elif defined  (STM32F2XX)
						FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR);	
						do i=FLASH_EraseSector(n, VoltageRange_3);	while(i==FLASH_BUSY);
#endif			
						if(i==FLASH_COMPLETE)
							return(0);
						else
							return(i);
}
/*******************************************************************************
* Function Name  : FlashProgram32
* Description    : programiranje  ali verificiranje 32 bitov, klièe  driver v knjižnici samo
*				 : èe se vsebina razlikuje od zahtevane; specifikacije enake kot FLASH_ProgramWord 
*				 : iz knjižnice
* Input          :
* Output         :
* Return         :
*******************************************************************************/
int					FlashProgram32(uint32_t Address, uint32_t Data) {
int					i;	
						if(!memcmp((const void *)Address,&Data,4))
							return(0);
						else {
							FLASH_Unlock();
#if defined (STM32F10X_HD)
							FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);						
							do i=FLASH_ProgramWord(Address,Data); while(i==FLASH_BUSY);
#elif defined  (STM32F2XX)
							FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR);	
							do i=FLASH_ProgramWord(Address,Data); while(i==FLASH_BUSY);
#endif				
						}
						if(i==FLASH_COMPLETE)
							return(0);
						else
							return(i);
}
/*******************************************************************************
* Function Name  : CanHexProg request, server
* Description    : dekodira in razbije vrstice hex fila na 	pakete 8 bytov in jih
*								 : pošlje na CAN bootloader
* Input          : pointer na string, zaporedne vrstice hex fila, <cr> <lf> ali <null> niso nujni
* Output         : 
* Return         : 0 ce je checksum error sicer eof(-1). bootloader asinhrono odgovarja z ACK message
*				 				 : za vsakih 8 bytov !!!
*******************************************************************************/
int					HexrecProg(char *p) {

int	 				xa=0,amax=0,amin=INT_MAX;
int	 				i,a,n;
union				{
							int i;
							char	c[4];
						} d;

						++p;
						n=str2hex(&p,2);																												// number of bytes
						a=(xa<<16)+str2hex(&p,4);																								// set address
						switch(str2hex(&p,2)) {
							case 00:																															// data record
								for(i=0; n--;) {
									if(!i)
										d.i=*(int *)a;																									// get the programmed int value
									d.c[i++]=str2hex(&p,2);
									if(i==4 || !n) {
										if(FlashProgram32(a,d.i))
											return 0;
										i=0;
										a+=sizeof(int);
									}
								}
								if(a<amin)
									amin=a;
								if(a>amax)
									amax=a;
								break;
							case 04:
								xa=str2hex(&p,4);	
								break;
						}
						return(amin);
}
/*******************************************************************************
* Function Name  : CanHexProg request, server
* Description    : dekodira in razbije vrstice hex fila na 	pakete 8 bytov in jih
*								 : pošlje na CAN bootloader
* Input          : pointer na string, zaporedne vrstice hex fila, <cr> <lf> ali <null> niso nujni
* Output         : 
* Return         : 0 ce je checksum error sicer eof(-1). bootloader asinhrono odgovarja z ACK message
*				 				 : za vsakih 8 bytov !!!
*******************************************************************************/							
int					HexrecOk(char *filename, FIL *f) {
int	 				xa=0,a,amax=0,amin=INT_MAX;
char				*p,s[64];
						if(f_open(f,filename,FA_READ)==FR_OK) {
							while(!f_eof(f)) {
								p=s;
								f_gets(p,64,f);
								if(*p != ':') {
									f_close(f);
									return 0;
								}
								if(HexChecksumError(++p)) {
									f_close(f);
									return 0;
								}
								a=(xa<<16)+str2hex(&p,4)+str2hex(&p,2);														// set address
								switch(str2hex(&p,2)) {
									case 00: 																												// data record
										if(a<amin)
											amin=a;
										if(a>amax)
											amax=a;
										break;
									case 04:
										xa=str2hex(&p,4);	
										break;
									default:
										break;
								}
							}
							if(amin == (int)__Vectors) {
								f_close(f);
								return 0;
							}
							f_lseek(f,0);
							return amin;
						}
						return NULL;
}
/*******************************************************************************/
int					HexrecSign(int a) {
int 				i,crc;
						RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_CRC, ENABLE);
						i=FlashErase(_SIGN_PAGE);
						CRC_ResetDR();
						crc=CRC_CalcBlockCRC((uint32_t *)a,0x8000);
						i |= FlashProgram32((int)_FW_CRC,crc);																// vpisi !!!
						i |= FlashProgram32((int)_FW_SIZE,0x8000);
						i |= FlashProgram32((int)_FW_START,a);
						CRC_ResetDR();
						crc=CRC_CalcBlockCRC((uint32_t *)_FW_SIZE,3);
						i |= FlashProgram32((int)_SIGN_CRC,crc);						
						return i;
}


