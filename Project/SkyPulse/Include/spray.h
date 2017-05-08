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

typedef	struct {
	bool	cEnabled:1;
	bool	On:1;
}	mode;

class	_SPRAY:public _ADC {
	private:
		int			Bottle_ref, Bottle_P;
		int			Air_ref, Air_P;
		int			WaterLeft,WaterMax,WaterMin;
		int			idx;
		_VALVE	*BottleIn,*BottleOut,*Air,*Water;

	public:
		_SPRAY();
		mode		mode;
		int			AirLevel, WaterLevel;
		int			Poll(void);
		void		LoadSettings(FILE *);
		void		SaveSettings(FILE *);
		void		Increment(int, int);
		bool		vibrate;
#ifdef __SIMULATION__
		void		Simulator(void);
		double	pComp,pAir,pBott,pAmb;
#endif

	};

#endif
	
