/**
  ******************************************************************************
  * @file  main.c
  * @author ...
  * @version  V1.0.0
  * @date  23/09/2011
  * @brief  This file provides main() entry to PFM6 application functions.
  ******************************************************************************
  *
  * <h2><center>&copy; COPYRIGHT 2011 Fotona d.d.</center></h2>
  */
/* Includes ------------------------------------------------------------------*/
#include		"iap.h"
#include		"io.h"
#include		"limits.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/
/* Includes ------------------------------------------------------------------*/
char				_Iap_string[_IAP_STRING_LEN];
int 				_Words32Received	=0,
						_minAddress				=INT_MAX,
						_timeout=3;
				
CanTxMsg		tx={_ID_IAP_ACK,0,CAN_ID_STD,CAN_RTR_DATA,1,0,0,0,0,0,0,0,0};
/******************************************************************************/
/** @addtogroup Main
  * @{
  */
/******************************************************************************/
int					main(void) {
int					*p=(int *)*_FW_START;
	
						Watchdog_init(4000);
						if(RCC_GetFlagStatus(RCC_FLAG_SFTRST) == SET && !crcError()) {
							NVIC_SetVectorTable(NVIC_VectTab_FLASH,(uint32_t)p-_BOOT_TOP);				
							__set_MSP(*p++);
							((void (*)(void))*p)();
						}
						App_Init();
	
#if	defined (__PFM6__) || defined(__DISCO__)
						if(RCC_GetFlagStatus(RCC_FLAG_WWDGRST) != RESET) {
							RCC_ClearFlag();	
							FileHexProg();
						}
#endif
						RCC_ClearFlag();	
						while(1)
							App_Loop();							
}
/******************************************************************************/
void				Watchdog_init(int t) {
						IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
						IWDG_SetPrescaler(IWDG_Prescaler_32);
						IWDG_SetReload(t);
						while(IWDG_GetFlagStatus(IWDG_FLAG_RVU) == RESET);
						IWDG_ReloadCounter();
						IWDG_Enable();
						IWDG_WriteAccessCmd(IWDG_WriteAccess_Disable);
}
/******************************************************************************/
void		 		Watchdog(void) {
						IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
						IWDG_ReloadCounter();
						IWDG_WriteAccessCmd(IWDG_WriteAccess_Disable);
}
/******************************************************************************/
void 				SysTick_init(void)
{
						RCC_ClocksTypeDef RCC_Clocks;
						RCC_GetClocksFreq(&RCC_Clocks);
						SysTick_Config(RCC_Clocks.HCLK_Frequency / 1000);
						NVIC_SetPriority(SysTick_IRQn, 0x03);
}
/*******************************************************************************
* Function Name  : Demo_Init
* Description    : Initializes the demonstration application.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
//char 				*leds[]={"d0","d1","d2","d3","","d4","d5","d6","d7"};

void 				App_Init(void) {

#if defined (__PVC__)
						RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOD|RCC_APB2Periph_GPIOE|RCC_APB2Periph_GPIOF|RCC_APB2Periph_GPIOG,ENABLE);				
#elif defined  (STM32F2XX)
						RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOC|RCC_AHB1Periph_GPIOD|RCC_AHB1Periph_GPIOE|RCC_AHB1Periph_GPIOF|RCC_AHB1Periph_GPIOG,ENABLE);	
#endif

						Initialize_CAN(0);							// 0=normal, 1=loopback(testiranje)
						SysTick_init();
						
#ifdef WITH_COM_PORT
						__stdin.handle.io=__stdout.handle.io=Initialize_USART();
						printf(IAP_MSG);
//						Initialize_LED(leds,9);
#endif
#if		defined (__PFM6__)
{
// pfm ventilatorji off		
						GPIO_InitTypeDef GPIO_InitStructure;   
						GPIO_StructInit(&GPIO_InitStructure);
						GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
						GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
						GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
						GPIO_Init(GPIOA, &GPIO_InitStructure);
						GPIO_ResetBits(GPIOA,GPIO_Pin_6);

// red, yellow, blue		
						GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
						GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_3;
						GPIO_Init(GPIOD, &GPIO_InitStructure);

						GPIO_SetBits(GPIOD,GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_3);
}			
#endif
						_Words32Received=0;
						CAN_Transmit(__CAN__,&tx);
}
/*******************************************************************************/
void				__App_Loop(void)  {

#ifdef WITH_COM_PORT
static int	t=0;
						if(t != __time__) {
							t=__time__;
							if(!(t % 100)) {
								GPIO_ToggleBits(GPIOD,GPIO_Pin_0);
								GPIO_SetBits(GPIOD,GPIO_Pin_2 | GPIO_Pin_3);
							}				
						if(t/1000 == _timeout)
							NVIC_SystemReset();
						}
//-------------------------------------------												
						ParseCOM();		
#endif
						ParseCAN(NULL);
						Watchdog();
}
/*******************************************************************************/
void				(*App_Loop)(void)= __App_Loop;
/*******************************************************************************
* Function Name  : FlashErase
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
#if defined (__PVC__)
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
*								 : èe se vsebina razlikuje od zahtevane; specifikacije enake kot FLASH_ProgramWord 
*								 : iz knjižnice
* Input          :
* Output         :
* Return         :
*******************************************************************************/
int					FlashProgram32(uint32_t Address, uint32_t Data) {
int					i;	
						if(!memcmp((const void *)Address,&Data,4))
							return(0);
						else if(Address < (uint32_t)_SIGN_CRC)
							return(-1);
						else {
							FLASH_Unlock();
#if defined (__PVC__)
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
- racuna  crc cez programski del, od BOOT page naprej do prve strani, ki ima na 
	koncu prazen prostor (memcmp z blank[]
- predhodno preskoci, ce je BOOT page prazen ze na zacetku
- izracuna crc
- ce je SIGN page prazen, vpise crc in vrne negativen odgovor
- ce SIGN page ni prazen primerja s CRC in vrne rezultat
********************************************************************************/
int					crcError(void) {
int 				i;

#if defined (__PVC__)
						RCC_AHBPeriphClockCmd(RCC_AHBPeriph_CRC, ENABLE);
#elif defined  (STM32F2XX)
						RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_CRC, ENABLE);
#endif
						CRC_ResetDR();
						if(CRC_CalcBlockCRC((uint32_t *)_FW_SIZE,3)==*_SIGN_CRC) {
							CRC_ResetDR();
							i=CRC_CalcBlockCRC((uint32_t *)*_FW_START,*_FW_SIZE);
							if(i== *_FW_CRC)																																						// zdaj morata bit enaka ....
								return 0;	
						}
						return(-1);
}
/*******************************************************************************/
int					crcSIGN(void) {
int 				i=-1,crc;

#if defined (__PVC__)
						RCC_AHBPeriphClockCmd(RCC_AHBPeriph_CRC, ENABLE);
#elif defined  (STM32F2XX)
						RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_CRC, ENABLE);
#endif				
#ifdef	__DISCO__	
						_Words32Received=0;
						_minAddress=0x08008000;
						
						if(_Words32Received)
#endif						
						{
							i=FlashErase(_SIGN_PAGE);
							CRC_ResetDR();
							crc=CRC_CalcBlockCRC((uint32_t *)_minAddress,_Words32Received);
							i |= FlashProgram32((int)_FW_CRC,crc);																							// vpisi !!!
							i |= FlashProgram32((int)_FW_SIZE,_Words32Received);
							i |= FlashProgram32((int)_FW_START,_minAddress);
							CRC_ResetDR();
							crc=CRC_CalcBlockCRC((uint32_t *)_FW_SIZE,3);
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
						GPIO_ToggleBits(GPIOD,GPIO_Pin_3);
						_timeout=-1;
						tx.StdId=_ID_IAP_ACK;
						tx.DLC=1;
						tx.Data[0]=a;
						while(CAN_Transmit(__CAN__,&tx)==CAN_NO_MB)
							App_Loop();
}					
/*******************************************************************************
* Function Name  : ParseCAN
* Description    : periodièno procesiranje CAN protokola v glavni zanki
* Input          : 
* Output         : 
* Return         : FLASH_COMPLETE na bootloader strani, FLASH_STATUS na strani 
* klienta (glej stm32f10x_flash.h)
*******************************************************************************/
int					ParseCAN(CanRxMsg *p) {

static int	addr,n=0;													// statièni register za zaèetno. adreso, index IAP stringa
int					i,ret=EOF;												// ....
CanRxMsg		rx;
						if(p)
							memcpy(&rx,p,sizeof(CanRxMsg));
						else {
							if(!CAN_MessagePending(__CAN__, CAN_FIFO0))
								return	ret;
							CAN_Receive(__CAN__,CAN_FIFO0, &rx);
							GPIO_ToggleBits(GPIOD,GPIO_Pin_2);
						} 

						switch(rx.StdId) {
//----------------------------------------------------------------------------------------------
// client - deep sleep (watchdog), no ack.
							case _ID_IAP_GO:
								NVIC_SystemReset();
								break;
//----------------------------------------------------------------------------------------------
// client - sign FW
							case _ID_IAP_SIGN:
								ret=crcSIGN();
								if(!p)
									SendAck(ret);
							break;
//----------------------------------------------------------------------------------------------
// client - setup adrese, no ack	
							case _ID_IAP_ADDRESS:						
								addr=*(int *)rx.Data;
								if(addr<_minAddress)
									_minAddress=addr;
							break;
//----------------------------------------------------------------------------------------------
// client - programiranje 2x4 bytov, ack
							case _ID_IAP_DWORD:	
								for(i=rx.DLC; i<8; ++i) 
									rx.Data[i]=((char *)addr)[i];
								ret=FlashProgram32(addr,*(int *)(&rx.Data[0]));
								addr+=4;
								++_Words32Received;
								if(rx.DLC>4) {
									ret |= FlashProgram32(addr,*(int *)(&rx.Data[4]));
								}
								addr+=4;
								++_Words32Received;
								if(!p)
									SendAck(ret);
								break;
//----------------------------------------------------------------------------------------------
// client - brisanje, ack	
							case _ID_IAP_ERASE:	
								_Words32Received=0;
								Watchdog();
								ret=FlashErase(*(int *)rx.Data);
								if(!p)
									SendAck(ret);
								break;	
//----------------------------------------------------------------------------------------------
// client - brisanje, ack	
							case _ID_IAP_STRING:
								for(i=0; i<rx.DLC && n<_IAP_STRING_LEN; ++i, ++n)
									_Iap_string[n]=rx.Data[i];
								if(_Iap_string[n-1]=='\0' || _Iap_string[n-1]=='\r' || _Iap_string[n-1]=='\n' || n==_IAP_STRING_LEN) {
									n=0;
									CanHexProg(NULL);
								}
							break;	
//----------------------------------------------------------------------------------------------
// client - brisanje, ack	
							case _ID_IAP_PING:
								ret=0;
								if(!p)
									SendAck(0);
							break;	
//----------------------------------------------------------------------------------------------
// server - acknowledge received
							case _ID_IAP_ACK:		
								ret=rx.Data[0];
								p=&rx;
							break;
//----------------------------------------------------------------------------------------------
							default:
							break;
						}
#ifdef WITH_COM_PORT
						if(p && ret != EOF) {
static int	dots=0;
							if(!(++dots % 32)) 
								printf("\r\n");
							if(ret)
								printf("?");
							else
								printf("-");
						}
#endif
						return(ret);
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
int					CanHexProg(char *s) {

static int	ExtAddr=0;
int	 				n,a,i=FLASH_COMPLETE;
int					*d=(int *)tx.Data;
char				*p;

						if(!s)
							p=_Iap_string;
						else
							p=s;
						if(HexChecksumError(p))
							return 0;
						n=str2hex(&p,2);
						a=(ExtAddr<<16)+str2hex(&p,4);
						switch(str2hex(&p,2)) {
							case 00:
								if(a<_FLASH_TOP)
									return 0;
								tx.StdId=_ID_IAP_ADDRESS;
								d[0]=a;
								tx.DLC=sizeof(int);

								if(!s)
									ParseCAN((CanRxMsg *)&tx);
								else
									while(CAN_Transmit(__CAN__,&tx)==CAN_NO_MB);

								tx.StdId=_ID_IAP_DWORD;
								for(i=0; n--;) {
									tx.Data[i++]=str2hex(&p,2);
									if(i==8 || !n) {
										tx.DLC=i;
										i=0;

									if(!s)
										ParseCAN((CanRxMsg *)&tx);
									else
										while(CAN_Transmit(__CAN__,&tx)==CAN_NO_MB);
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
/*******************************************************************************
* Function Name  : CanHexString request, server
* Description    : pošlje string na CAN bootloader z id. _ID_IAP_STRING _
*								 : * Input: pointer na string, zaporedne vrstice hex fila, 
*								 :<cr> <lf> ali <null> niso nujni
* Output         : 
* Return         : vraca stevilo prenesenih bytov
*******************************************************************************/
int					CanHexString(char *p) {
int					i=0;

						if(!p)
							p=_Iap_string;
						tx.StdId=_ID_IAP_STRING;
						do {
							do {
								tx.Data[i%8]=p[i];
								if(!p[i++])
									break;
							} while(i%8);
							if(i%8)
								tx.DLC=i%8;
							else
								tx.DLC=8;
								
							if(p==_Iap_string)
								ParseCAN((CanRxMsg *)&tx);
							else
								while(CAN_Transmit(__CAN__,&tx)==CAN_NO_MB)
									App_Loop();
							
						} while(p[i-1]);
						return(i);
}
/*******************************************************************************
* Function Name  : CanHexMessage request, server
* Description    : briše flash 
* Input          : parameter m = _ID_IAP_ERASE, _ID_IAP_SIGN, _ID_IAP_GO
* Output         : 
* Return         : ni, po koncu bootloader asinhrono odgovori z ACK message
*******************************************************************************/
void				CanHexMessage(char m, int n) {
						tx.StdId=m;
						tx.DLC=sizeof(int);
						*(int *)tx.Data=n;
						while(CAN_Transmit(__CAN__,&tx)==CAN_NO_MB)
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
* Function Name  : str2hex
* Description    : pretvorba stringa v hex stevilo
* Input          : **p, pointer na string array (char *), n stevilo znakov
* Output         : char * se poveca za stevilo znakov
* Return         : hex integer
*******************************************************************************/
int					str2hex(char **p,int n) {
char				q[16];
						strncpy(q,*p,n)[n]='\0';
						*p += n;
						return(strtoul(q,NULL,16));
}
/*******************************************************************************
* Function Name  : CAN_Initialize
* Description    : 
* Input          : None
* Output         : None
* Return         : 
*******************************************************************************/
void 				Initialize_CAN(int loop) {
#if defined (__PVC__)
GPIO_InitTypeDef			GPIO_InitStructure;
CAN_InitTypeDef				CAN_InitStructure;
CAN_FilterInitTypeDef	CAN_FilterInitStructure;

						CAN_StructInit(&CAN_InitStructure);
						RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO |RCC_APB2Periph_GPIOA, ENABLE);  //pppp dodal

						RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);
						CAN_DeInit(__CAN__); 
						CAN_InitStructure.CAN_TTCM=DISABLE;
						CAN_InitStructure.CAN_ABOM=ENABLE;
						CAN_InitStructure.CAN_AWUM=DISABLE;
						CAN_InitStructure.CAN_NART=ENABLE;
						CAN_InitStructure.CAN_RFLM=DISABLE;
						CAN_InitStructure.CAN_TXFP=DISABLE;
						if(loop)
							CAN_InitStructure.CAN_Mode=CAN_Mode_LoopBack;
						else
							CAN_InitStructure.CAN_Mode=CAN_Mode_Normal;
// ... pomembn.. da ne zamesa mailboxov in jih oddaja po vrstnem redu vpisovanja... ni default !!!
						CAN_InitStructure.CAN_TXFP=ENABLE;	
						CAN_InitStructure.CAN_SJW=CAN_SJW_4tq;
						CAN_InitStructure.CAN_BS1=CAN_BS1_12tq;
						CAN_InitStructure.CAN_BS2=CAN_BS2_5tq;
						CAN_InitStructure.CAN_Prescaler=4;
						CAN_Init(__CAN__,&CAN_InitStructure);
						
						CAN_FilterInitStructure.CAN_FilterMode=CAN_FilterMode_IdList;
						CAN_FilterInitStructure.CAN_FilterScale=CAN_FilterScale_32bit;
						CAN_FilterInitStructure.CAN_FilterMaskIdLow=0;
						CAN_FilterInitStructure.CAN_FilterIdLow=0;
						CAN_FilterInitStructure.CAN_FilterActivation=ENABLE;
						
						GPIO_StructInit(&GPIO_InitStructure);
						
// // ... RX pin
// 						GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;	//GPIO_Pin_11;
// 						GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
// 						GPIO_Init(GPIOB, &GPIO_InitStructure);
// // ... TX pin					
// 						GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //GPIO_Pin_12;
// 						GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
// 						GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
// 						GPIO_Init(GPIOB, &GPIO_InitStructure);

// ... RX pin
						GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
						GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
						GPIO_Init(GPIOA, &GPIO_InitStructure);
// ... TX pin
						GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
						GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
						GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
						GPIO_Init(GPIOA, &GPIO_InitStructure); 

// filtri
						CAN_FilterInitStructure.CAN_FilterMode=CAN_FilterMode_IdList;
						CAN_FilterInitStructure.CAN_FilterScale=CAN_FilterScale_32bit;
						
						CAN_FilterInitStructure.CAN_FilterMaskIdLow=0;
						CAN_FilterInitStructure.CAN_FilterIdLow=0;
						CAN_FilterInitStructure.CAN_FilterFIFOAssignment=CAN_FIFO0;
						CAN_FilterInitStructure.CAN_FilterActivation=ENABLE;
// nastavitev adrese & brisanje
						CAN_FilterInitStructure.CAN_FilterIdHigh=_ID_IAP_ADDRESS<<5;
						CAN_FilterInitStructure.CAN_FilterMaskIdHigh=_ID_IAP_ERASE<<5;
						CAN_FilterInitStructure.CAN_FilterNumber=0;
						CAN_FilterInit(&CAN_FilterInitStructure);
// programiranje 32 bit, acknowledge
						CAN_FilterInitStructure.CAN_FilterIdHigh=_ID_IAP_DWORD<<5;
						CAN_FilterInitStructure.CAN_FilterMaskIdHigh=_ID_IAP_ACK<<5;
						CAN_FilterInitStructure.CAN_FilterNumber=1;
						CAN_FilterInit(&CAN_FilterInitStructure);
// 	run
						CAN_FilterInitStructure.CAN_FilterIdHigh=_ID_IAP_GO<<5;
						CAN_FilterInitStructure.CAN_FilterMaskIdHigh=_ID_IAP_SIGN<<5;
						CAN_FilterInitStructure.CAN_FilterNumber=2;
						CAN_FilterInit(&CAN_FilterInitStructure);
						
						CAN_FilterInitStructure.CAN_FilterIdHigh=_ID_IAP_STRING<<5;
						CAN_FilterInitStructure.CAN_FilterMaskIdHigh=_ID_IAP_PING<<5;
						CAN_FilterInitStructure.CAN_FilterNumber=3;
						CAN_FilterInit(&CAN_FilterInitStructure);
						
#elif defined  (STM32F2XX)
CAN_InitTypeDef					CAN_InitStructure;
CAN_FilterInitTypeDef		CAN_FilterInitStructure;
GPIO_InitTypeDef				GPIO_InitStructure;

						GPIO_StructInit(&GPIO_InitStructure);
						GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
						GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

#ifdef			__PFM6__
						GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
						GPIO_Init(GPIOB, &GPIO_InitStructure);
						GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
						GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
						GPIO_Init(GPIOB, &GPIO_InitStructure);

						GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_CAN2);
						GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_CAN2);
#else
						GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
						GPIO_Init(GPIOA, &GPIO_InitStructure);
						GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
						GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
						GPIO_Init(GPIOA, &GPIO_InitStructure);

						GPIO_PinAFConfig(GPIOA, GPIO_PinSource11, GPIO_AF_CAN1);
						GPIO_PinAFConfig(GPIOA, GPIO_PinSource12, GPIO_AF_CAN1);
#endif
						RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);
						RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN2, ENABLE);
						CAN_StructInit(&CAN_InitStructure);
						CAN_DeInit(__CAN__);
						CAN_InitStructure.CAN_TTCM=DISABLE;
						CAN_InitStructure.CAN_ABOM=ENABLE;
						CAN_InitStructure.CAN_AWUM=DISABLE;
						CAN_InitStructure.CAN_NART=ENABLE;
						CAN_InitStructure.CAN_RFLM=DISABLE;
//... pomembn.. da ne zamesa mailboxov in jih oddaja po vrstnem redu vpisovanja... ni default !!!
						CAN_InitStructure.CAN_TXFP=ENABLE;	
						if(loop)
							CAN_InitStructure.CAN_Mode=CAN_Mode_LoopBack;
						else
							CAN_InitStructure.CAN_Mode=CAN_Mode_Normal;

						CAN_InitStructure.CAN_SJW=CAN_SJW_4tq;
						CAN_InitStructure.CAN_BS1=CAN_BS1_10tq;
						CAN_InitStructure.CAN_BS2=CAN_BS2_4tq;
						CAN_InitStructure.CAN_Prescaler=4;
						CAN_Init(__CAN__,&CAN_InitStructure);

						CAN_FilterInitStructure.CAN_FilterMode=CAN_FilterMode_IdList;
						CAN_FilterInitStructure.CAN_FilterScale=CAN_FilterScale_32bit;
						CAN_FilterInitStructure.CAN_FilterMaskIdLow=0;
						CAN_FilterInitStructure.CAN_FilterIdLow=0;
						CAN_FilterInitStructure.CAN_FilterActivation=ENABLE;

						CAN_FilterInitStructure.CAN_FilterFIFOAssignment=CAN_FIFO0;

						CAN_FilterInitStructure.CAN_FilterIdHigh=_ID_IAP_DWORD<<5;
						CAN_FilterInitStructure.CAN_FilterMaskIdHigh=_ID_IAP_ADDRESS<<5;
						CAN_FilterInitStructure.CAN_FilterNumber=__FILTER_BASE__+0;
						CAN_FilterInit(&CAN_FilterInitStructure);

						CAN_FilterInitStructure.CAN_FilterIdHigh=_ID_IAP_GO<<5;
						CAN_FilterInitStructure.CAN_FilterMaskIdHigh=_ID_IAP_ERASE<<5;
						CAN_FilterInitStructure.CAN_FilterNumber=__FILTER_BASE__+1;
						CAN_FilterInit(&CAN_FilterInitStructure);

						CAN_FilterInitStructure.CAN_FilterIdHigh=_ID_IAP_ACK<<5;
						CAN_FilterInitStructure.CAN_FilterMaskIdHigh=_ID_IAP_SIGN<<5;
						CAN_FilterInitStructure.CAN_FilterNumber=__FILTER_BASE__+2;
						CAN_FilterInit(&CAN_FilterInitStructure);
						
						CAN_FilterInitStructure.CAN_FilterIdHigh=_ID_IAP_STRING<<5;
						CAN_FilterInitStructure.CAN_FilterMaskIdHigh=_ID_IAP_PING<<5;
						CAN_FilterInitStructure.CAN_FilterNumber=__FILTER_BASE__+3;
						CAN_FilterInit(&CAN_FilterInitStructure);
#endif
}
/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
volatile 
int 				__time__;
void 				SysTick_Handler(void) {
						++__time__;
}
#if	defined (__PFM6__) || defined(__DISCO__)
/******************************************************************************/
void				FileHexProg(void) {
FATFS	fs;
FIL		f;
int		i;
char	s[128];

						if(f_mount(0,&fs)==FR_OK && f_open(&f,"Pfm6Ctrl.hex",FA_READ)==FR_OK) {
							for(i=0; i<5; ++i) {
								Watchdog();
								FlashErase(_SIGN_PAGE+i*_PAGE_SIZE);
							}
							while(!f_eof(&f)) {
								Watchdog();
								f_gets(s,128,&f);
								strncpy(_Iap_string,&s[1],63);
								CanHexProg(NULL);
							}
							crcSIGN();
						}
						f_close(&f);
						f_mount(0,NULL);
}
#endif
/**
* @}
*/ 
