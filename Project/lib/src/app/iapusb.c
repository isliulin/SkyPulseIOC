#include		"pfm.h"

#define 		_FW_CRC				((uint32_t *)(_FLASH_TOP-4))
#define 		_FW_SIZE			((uint32_t *)(_FLASH_TOP-8))
#define 		_SIGN_CRC			((uint32_t *)(_FLASH_TOP-12))
#define			_FLASH_BLANK	((uint32_t)-1)

char				_Iap_string[_IAP_STRING_LEN];
int 				_Words32Received;
int					CanHexProg(char *);
int					FlashProgram32(uint32_t , uint32_t );
int					EraseFLASH(int );
/*******************************************************************************/
int					crcSIGN(void) {
int 				i=-1,crc;

#if defined (STM32F10X_HD)
						RCC_AHBPeriphClockCmd(RCC_AHBPeriph_CRC, ENABLE);
#elif defined  (STM32F2XX)
						RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_CRC, ENABLE);
#endif						
						if(_Words32Received) {
							i=EraseFLASH(_SIGN_PAGE);
							CRC_ResetDR();
							crc=CRC_CalcBlockCRC((uint32_t *)_FLASH_TOP,_Words32Received);
							i |= FlashProgram32((int)_FW_CRC,crc);																							// vpisi !!!
							i |= FlashProgram32((int)_FW_SIZE,_Words32Received);
							CRC_ResetDR();
							crc=CRC_CalcBlockCRC(_FW_SIZE,2);
							i |= FlashProgram32((int)_SIGN_CRC,crc);						
						}
						return i;
}

/*******************************************************************************
* Function Name  : SendAck
* Description    : odda _ID_IAP_ACK message 
* Input          : 
* Output         : 
* Return         :
*******************************************************************************/
void				SendAck(int a) {
CanTxMsg		Tx;
						Tx.RTR=CAN_RTR_DATA;
						Tx.IDE=CAN_ID_STD;
						Tx.StdId=_ID_IAP_ACK;
						Tx.DLC=1;
						Tx.Data[0]=a;
						while(CAN_Transmit(CAN1,&Tx)==CAN_NO_MB)
							App_Loop();
}			
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
						for(err=n;n-->-5;err+=str2hex(&p,2));
						return(err & 0xff);
}
/*******************************************************************************
* Function Name  : EraseFLASH
* Description    : Brisanje flash bloka
* Input          :
* Output         :
* Return         :
*******************************************************************************/
int					EraseFLASH(int n) {
int					i;
						if(n == _BOOT_SECTOR) 
							return(-1);
#if defined (STM32F10X_HD)
						do i=FLASH_ErasePage(n); while(i==FLASH_BUSY);
#elif defined  (STM32F2XX)
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
						else
							do i=FLASH_ProgramWord(Address,Data); while(i==FLASH_BUSY);
						if(i==FLASH_COMPLETE)
							return(0);
						else
							return(i);
}
/*******************************************************************************
* Function Name  : PollCAN
* Description    : periodièno procesiranje CAN protokola v glavni zanki
* Input          : 
* Output         : 
* Return         : FLASH_COMPLETE na bootloader strani, FLASH_STATUS na strani 
* klienta (glej stm32f10x_flash.h)
*******************************************************************************/
int					PollCAN(CanRxMsg *p) {

static int	addr,n=0;									// statièni register za zaèetno. adreso, index IAP stringa
int					i;												// ....
CanRxMsg		Rx;

						if(!p) {
							if(!CAN_MessagePending(CAN1, CAN_FIFO0))
								return(EOF);
							p=&Rx;
							CAN_Receive(CAN1,CAN_FIFO0, p);
						}
						switch(p->StdId) {
//----------------------------------------------------------------------------------------------
// client - deep sleep (watchdog), no ack.
							case _ID_IAP_GO:
								NVIC_SystemReset();
								break;
//----------------------------------------------------------------------------------------------
// client - sign FW
							case _ID_IAP_SIGN:
								SendAck(crcSIGN());
							break;
//----------------------------------------------------------------------------------------------
// client - setup adrese, no ack	
							case _ID_IAP_ADDRESS:						
								addr=*(int *)p->Data;
							break;
//----------------------------------------------------------------------------------------------
// client - programiranje 2x4 bytov, ack
							case _ID_IAP_DWORD:	
								for(i=p->DLC; i<8; ++i) 
									p->Data[i]=((char *)addr)[i];
								i=FlashProgram32(addr,*(int *)(&p->Data[0]));
								addr+=4;
								++_Words32Received;
								if(p->DLC>4) {
									i |= FlashProgram32(addr,*(int *)(&p->Data[4]));
								}
								addr+=4;
								++_Words32Received;
								SendAck(i);
								break;
//----------------------------------------------------------------------------------------------
// client - brisanje, ack	
							case _ID_IAP_ERASE:	
								_Words32Received=0;
								Watchdog();	
								SendAck(EraseFLASH(*(int *)p->Data));
								break;	
//----------------------------------------------------------------------------------------------
// client - brisanje, ack	
							case _ID_IAP_STRING:
								for(i=0; i<p->DLC && n<_IAP_STRING_LEN; ++i, ++n)
									_Iap_string[n]=p->Data[i];
								if(_Iap_string[n-1]=='\0' || _Iap_string[n-1]=='\r' || _Iap_string[n-1]=='\n' || n==_IAP_STRING_LEN) {
									n=0;
									CanHexProg(NULL);
								}
							break;	
//----------------------------------------------------------------------------------------------
// client - brisanje, ack	
							case _ID_IAP_PING:
								SendAck(0);
							break;	
//----------------------------------------------------------------------------------------------
// server - acknowledge received
							case _ID_IAP_ACK:						
								return(p->Data[0]);
//----------------------------------------------------------------------------------------------
							default:
							break;
						}
						return(EOF);
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
int					CanHexProg(char *p) {

static int	ExtAddr=0;
int	 				n,a,i=FLASH_COMPLETE;
CanTxMsg		TxMessage;
int					*d=(int *)TxMessage.Data;

						if(!p)
							p=_Iap_string;
						if(HexChecksumError(p))
							return(0);

						TxMessage.RTR=CAN_RTR_DATA;
						TxMessage.IDE=CAN_ID_STD;

						n=str2hex(&p,2);
						a=(ExtAddr<<16)+str2hex(&p,4);
						switch(str2hex(&p,2)) {
							case 00:
								TxMessage.StdId=_ID_IAP_ADDRESS;
								d[0]=a;
								TxMessage.DLC=sizeof(int);

								if(p==_Iap_string)
									PollCAN((CanRxMsg *)&TxMessage);
								else
									while(CAN_Transmit(CAN1,&TxMessage)==CAN_NO_MB);

								TxMessage.StdId=_ID_IAP_DWORD;
								for(i=0; n--;) {
									TxMessage.Data[i++]=str2hex(&p,2);
									if(i==8 || !n) {
										TxMessage.DLC=i;
										i=0;

									if(p==_Iap_string)
										PollCAN((CanRxMsg *)&TxMessage);
									else
										while(CAN_Transmit(CAN1,&TxMessage)==CAN_NO_MB);
									}	
								}
								break;
							case 01:
								break;
							case 02:
								break;
							case 04:
							case 05:
								ExtAddr=str2hex(&p,4);
								break;
						}
						return(EOF);
}
