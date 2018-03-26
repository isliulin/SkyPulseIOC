#ifndef		SIMULATOR_H
#define		SIMULATOR_H
#include	"adc.h"
#include	"lcd.h"

class _SIMULATOR : public _ADC {
private:
	_PLOT<unsigned short>  plot;	
	int		rate;
	float	pComp,
				pBott,
				pNozz,
				Pin,
				Pout,
				tau1,
				tau2;
public:
	_SIMULATOR();
	~_SIMULATOR();
	_LCD *lcd;

	void Poll(void *);
};
#endif
	
