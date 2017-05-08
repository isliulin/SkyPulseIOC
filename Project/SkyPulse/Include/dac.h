#ifndef					DAC_H
#define					DAC_H
#include				"stm32f2xx.h"
#include				"stdio.h"

class	_DAC {
	private:
	static _DAC *instance;
	public:
		_DAC();
};

extern unsigned short DacBuff[100];
void DAC_Ch2_Config(void);
void TIM6_Config(void);

#endif
	
