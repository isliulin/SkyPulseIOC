#ifndef					GPIO_H
#define					GPIO_H
#include				"stm32f2xx.h"

#define	__FOOT_MASK	0xf800

//
// Footswitch port pattern
//

typedef enum {							// gpioc	15 14 13 20 10
	__FOOT_OFF	=0xf800,	//				 1  1  1  1  1
	__FOOT_IDLE	=0x3800,	//				 0  0  1  1  1
	__FOOT_MID	=0xb800,	//				 1  0  1  1  1
	__FOOT_ON		=0xd800,	//				 1  1  0  1  1
	__FOOT_ACK	=0xffff		//
} __FOOT;

class	_GPIO { 
	private:
		int	key,
				temp,
				timeout;
	public:
		_GPIO(void);
		int	Poll(void);
};
#endif
