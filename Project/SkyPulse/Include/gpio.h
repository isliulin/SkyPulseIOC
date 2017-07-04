#ifndef					GPIO_H
#define					GPIO_H
#define	__FOOT_MASK	0xe000

//
// Footswitch port pattern
//

typedef enum {					// gpioc	15 14 13 20 10
	__FOOT_OFF	=0xe000,	//				 1  1  1  x  x
	__FOOT_1		=0x2000,	//				 0  0  1  x  x
	__FOOT_2		=0xa000,	//				 1  0  1  x  x
	__FOOT_3		=0x8000,	//				 1  0  0  x  x
	__FOOT_4		=0xc000		//				 1  1  0  x  x
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
