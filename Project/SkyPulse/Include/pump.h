#ifndef					PUMP_H
#define					PUMP_H
#include				"stdio.h"
#include				"stdlib.h"
#include				"adc.h"
#include				"dac.h"
#include				"tim.h"
#include				"fit.h"

typedef enum		{PUMP_FLOW, PUMP_ERR_STOP} _MODE_;

class	_PUMP:public _ADC,_DAC,_TIM3 {
	private:

int		idx,led,timeout,mode;
int		fpl,fph,ftl,fth;
_FIT	*tacho,*pressure,*current;
	
	public:
_PUMP();

int		curr_limit;
int		Poll(void);
int		Rpm(void);
int		Increment(int, int);
void	LoadSettings(FILE *);
void	SaveSettings(FILE *);
void	Enable(void),Disable(void);
bool	Align(void);
void	LoadLimits(FILE *);
void	SaveLimits(FILE *);
bool	Test(void);
};

#endif
