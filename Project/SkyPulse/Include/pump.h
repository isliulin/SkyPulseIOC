#ifndef					PUMP_H
#define					PUMP_H
#include				"stdio.h"
#include				"stdlib.h"
#include				"adc.h"
#include				"dac.h"
#include				"tim.h"
#include				"fit.h"

typedef enum		{PUMP_FLOW, PUMP_ERR_STOP} _MODE_;
#ifdef __IOC_V2__
class	_PUMP: public _ADC, public _TIM9, _DAC {
#else
class	_PUMP: public _ADC, public _TIM3, _DAC {
#endif
	private:

int		idx,led,timeout,mode;
int		fpl,fph,ftl,fth;
int		curr_limit,flow_limit,flow;
	public:
_PUMP();
int		Poll(void);
int		Rpm(void);
void	Increment(int);
int		Increment(int, int);
void	LoadSettings(FILE *);
void	SaveSettings(FILE *);
void	Enable(void),Disable(void);
bool	Enabled;
};

#endif
