#ifndef			FAN_H
#define			FAN_H
#include		"stdio.h"
#include		"adc.h"
#include		"dac.h"
#include		"tim.h"
#include		"fit.h"
#include		"isr.h"

#ifdef __IOC_V2__
	#define _fTIM	TIM10
class	_FAN: public _ADC, public _TIM9 {
#else
	#define _fTIM	TIM4
class	_FAN: public _ADC, public _TIM3 {
#endif

	private:
int		idx,timeout;
int		fpl, fph, ftl, fth;
int		tacho_limit,tacho;	
	public:
		_FAN();

int		Poll(void);
int		Rpm(void);
void	Increment(int);
void	Increment(int, int);
void	LoadSettings(FILE *);
void	SaveSettings(FILE *);
void	Enable(void),Disable(void);
};

#endif
