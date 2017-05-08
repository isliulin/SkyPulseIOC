#ifndef					INT_H
#define					INT_H
#include				"stm32f2xx.h"
#include				<stdio.h>

class	_INT {
	private:
		int Ref;
		int	Gain;
		int	Value;
		int	*Src;
		int	*Dst;
		
	public:
		_INT();
		int	Poll(void *v);

	};
#endif
	
