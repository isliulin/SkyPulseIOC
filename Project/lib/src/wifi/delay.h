#ifndef __DELAY_H
#define __DELAY_H

#if defined  (STM32F2XX)
#include		"stm32f2xx.h"
#elif defined (STM32F10X_HD)
#include		"stm32f10x.h"
#elif	undefined (STM32F2XX || STM32F10X_HD)
*** undefined target !!!!
#endif

#include	"sn8200_api.h"
#include	"sn8200_core.h"
#include	"io.h"

void 			SysTick_init(void);
void 			mdelay(uint32_t ms);
void			Watchdog(void);

#endif
extern
char			*strDbg;
int				DbgScan(const char *, ...);
int 			DbgPrint(const char *, ...);
extern
_io 			*__com0,*__com1;

void			Wait(int,void (*)(void));
extern 		char	*strDbgScan;
extern 
void			(*App_Loop)(void);
int				__sendFromSock(void);
