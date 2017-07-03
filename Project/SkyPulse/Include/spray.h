#ifndef					SPRAY_H
#define					SPRAY_H

#include				"stm32f2xx.h"
#include				<stdio.h>
#include				"isr.h"
#include				"dac.h"
#include				"adc.h"
#include				"tim.h"
#include				"lcd.h"
#include				"spray.h"

#define		_BAR(a) (a*0x4000)
extern void Simulate(void);					

typedef	struct {
	bool	cEnabled:1;
	bool	On:1;
}	mode;

class	_SPRAY:public _ADC {
	private:
		int			Bottle_ref, Bottle_P;
		int			Air_ref, Air_P;
		int			idx;

	public:
		_SPRAY();
		mode		mode;
		_VALVE	*BottleIn,*BottleOut,*Air,*Water;
		int			AirLevel, WaterLevel;
		int			Poll(void);
		void		LoadSettings(FILE *);
		void		SaveSettings(FILE *);
		void		Increment(int, int);
		bool		vibrate;
	
#ifdef __SIMULATION__	
		bool		Simulator(void);
		static double	pComp,pBott,pAir,pAmb;
		int			simrate;
#endif
};

#endif
	
