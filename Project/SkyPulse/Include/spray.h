#ifndef					SPRAY_H
#define					SPRAY_H

#include				<stdio.h>
#include				"isr.h"
#include				"dac.h"
#include				"adc.h"
#include				"tim.h"
#include				"lcd.h"
#include				"ioc.h"
#include				"spray.h"

#define					_SPRAY_READY_T	500
extern void			Simulate(void);					

typedef	struct {
	bool	Air:1;
	bool	Water:1;
	bool	Vibrate:1;
	bool	Simulator:1;
}	mode;

class	_SPRAY:public _ADC {
	private:
		int			Bottle_ref, Bottle_P;
		int			Air_ref, Air_P;
		int			idx,simrate;

	public:
		_SPRAY();
		mode		mode;
		int			waterGain;

		_VALVE	*BottleIn,*BottleOut,*Air,*Water;
		int			AirLevel, WaterLevel, readyTimeout, offsetTimeout;
		int			Poll(void);
		void		LoadSettings(FILE *);
		void		SaveSettings(FILE *);
		void		Increment(int, int);
	
		_LCD		*lcd;
		_PLOT<unsigned short>  plot;	
	
		bool		Simulator(void);
		double	pComp,pBott,pNozz,Pin,Pout;
};

#endif
	
