#if defined	(STM32F2XX)
#include		"stm32f2xx.h"

#define 		SW_version					110		

#if defined	(__PFM6__)
	#define			__CAN__						CAN2
	#define			__FILTER_BASE__		14
#elif defined	(__DISCO__)
	#define			__CAN__						CAN2
	#define			__FILTER_BASE__		14
#endif

#define			_PAGE_SIZE					FLASH_Sector_1
#define			_SIGN_PAGE					FLASH_Sector_1
#define			_FLASH_TOP					0x08008000
#define			_BOOT_TOP						0x08000000
#define			_BOOT_SECTOR				FLASH_Sector_0
#define			_IAP_STRING_LEN			128
		                            
#define			ERASE_SIZE					0x20000
#define			ERASE_COUNT					6
																
#define			STORAGE_TOP					0x8040000
/*---------------------------------------------*/
		                            
#elif defined (__PVC__)		      
#include		"stm32f10x.h"		    
		                            
#define			__CAN__							CAN1
#define			__FILTER_BASE__			0
		                            
#define			_PAGE_SIZE					0x800
#define			_SIGN_PAGE					(_FLASH_TOP-_PAGE_SIZE)
#define			_FLASH_TOP					0x08004000
#define			_BOOT_TOP						0x08000000
#define			_BOOT_SECTOR				_BOOT_TOP
#define			_IAP_STRING_LEN			128

#else
*** undefined target !!!!
#endif

int					FlashProgram32(uint32_t, uint32_t);
int					FlashErase(int);

#define 		_FW_START			((int *)(_FLASH_TOP-16))
#define 		_FW_CRC				((int *)(_FLASH_TOP-20))
#define 		_FW_SIZE			((int *)(_FLASH_TOP-24))
#define 		_SIGN_CRC			((int *)(_FLASH_TOP-28))
#define			_FLASH_BLANK	((int)-1)

#include		<stdio.h>
#include		<ctype.h>
#include		<stdlib.h>
#include		<string.h>
#include		"io.h"

#define			IAP_MSG "\r1>ERASE 2>SIGN 3>RUN\r\n"

#define			_ID_IAP_GO			0xA0
#define			_ID_IAP_ERASE		0xA1
#define			_ID_IAP_ADDRESS	0xA2
#define			_ID_IAP_DWORD		0xA3
#define			_ID_IAP_ACK			0xA4
#define			_ID_IAP_SIGN		0xA5
#define			_ID_IAP_STRING	0xA6
#define			_ID_IAP_PING		0xA7

#define			_CtrlY					0x19
#define			_CtrlZ					0x1A
#define			_Esc						0x1B
#define			_Eof						-1

int					crcError(void);
int					crcSIGN(void);
int	 				ParseCAN(CanRxMsg *);
int					HexChecksumError(char *);
int					CanHexProg(char *);
int					str2hex(char **,int);
void				Wait(int,void (*)(void));
void				Initialize_LED(char *[], int);

void 				SysTick_Configuration(void);
extern 
void				(*App_Loop)(void);
extern
char				_Iap_string[];
extern			uint32_t	__Vectors[];

void				App_Init(void);
void				Watchdog_init(int);
void		 		Watchdog(void);
void				WiFi(void);
void				CanHexMessage(char, int);
int					CanHexString(char *);
void				FileHexProg(void);
void				SendAck(int);
void				ParseCOM(void);
void 				Initialize_CAN(int);
_io*				Initialize_USART(void);
char				*cgets(int, int);

extern		
volatile 		int __time__;
