#ifndef					TOUCH_H
#define					TOUCH_H
#include				"stm32f2xx.h"
#include				<stdio.h>
#include 				<string.h>

class	_TOUCH { 
	private:
		int		n,x,y,t;	

	
	public:
		_TOUCH(void);
		int Poll(void);
};

#endif
