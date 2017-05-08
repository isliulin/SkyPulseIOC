#if defined  (STM32F2XX)
#include		"stm32f2xx.h"

#define			_PAGE_SIZE			0x4000
#define			_FLASH_TOP			0x08008000
#define			_BOOT_TOP				0x08000000
#define			_BOOT_SECTOR		_BOOT_TOP
#define			_SIGN_PAGE			FLASH_Sector_1
#define			_IAP_STRING_LEN	128

#elif defined (STM32F10X_HD)
#include		"stm32f10x.h"

#define			_PAGE_SIZE			0x800
#define			_FLASH_TOP			0x08004000
#define			_BOOT_TOP				0x08000000
#define			_BOOT_SECTOR		_BOOT_TOP
#define			_SIGN_PAGE			_FLASH_TOP-_PAGE_SIZE
#define			_IAP_STRING_LEN	128

#elif	undefined (STM32F2XX || STM32F10X_HD)
*** undefined target !!!!
#endif

#include		<stdio.h>
#include		<ctype.h>
#include		<stdlib.h>
#include		<string.h>

#define			_ID_IAP_GO			0xA0
#define			_ID_IAP_ERASE		0xA1
#define			_ID_IAP_ADDRESS	0xA2
#define			_ID_IAP_DWORD		0xA3
#define			_ID_IAP_ACK			0xA4
#define			_ID_IAP_SIGN		0xA5
#define			_ID_IAP_STRING	0xA6

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

void				App_Init(void);
void				App_Loop(void);
void				CanHexMessage(char, int);
int					CanHexString(char *);
void				SendAck(int);
void				ParseCOM(void);
void 				Initialize_CAN(int);
void				Initialize_USART(void);
char				*cgets(int, int);
