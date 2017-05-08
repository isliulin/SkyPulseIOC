#ifndef					EE_H
#define					EE_H

#include				"stm32f2xx.h"
#include				<stdio.h>
#include 				<stdlib.h>
#include				"isr.h"
#include				"adc.h"

#define					EE_PORT	GPIOA
#define					EE_BIT	GPIO_Pin_0

#define	_tRD		4
#define _tMRS		5
#define _tRCV		6
#define _tRESET	500
#define _tRRT		10
#define _tDDR		2

class	_EE {
	private:
		int		nbits, temp, phase;
		enum	{_IDLE,_BUSY,_RESET} status;

	public:
		_EE();
		~_EE();
		void			ISR(_EE *);
		char			*Exchg(char *);
		char			*getSerial(char *);	
		char			*getPage(int, char *);
		char			*putPage(int, char *);
};

#endif
