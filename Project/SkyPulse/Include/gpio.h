#ifndef					GPIO_H
#define					GPIO_H

#if defined   (__IOC_V0__)
	#define _12Voff_PIN GPIO_Pin_3
	#define _12Voff_PORT GPIOB
	#define _SYS_SHG_PIN 	GPIO_Pin_4
	#define _SYS_SHG_PORT GPIOB
	#define _SYS_SHG_sense_PIN 	GPIO_Pin_8
	#define _SYS_SHG_sense_PORT GPIOA
	#define _PILOT_PIN 	GPIO_Pin_13
	#define _PILOT_PORT GPIOD
	#define _FOOT_MASK 	GPIO_Pin_15 | GPIO_Pin_14 | GPIO_Pin_13
	#define _FOOT_PORT GPIOC
#elif defined  (__IOC_V1__) || defined(__DISCO__) 
	#define _12Voff_PIN GPIO_Pin_3
	#define _12Voff_PORT GPIOB
	#define _SYS_SHG_PIN GPIO_Pin_4
	#define _SYS_SHG_PORT GPIOB
	#define _SYS_SHG_sense_PIN 	GPIO_Pin_8
	#define _SYS_SHG_sense_PORT GPIOA
	#define _PILOT_PIN 	GPIO_Pin_13
	#define _PILOT_PORT GPIOD
	#define _FOOT_MASK 	GPIO_Pin_15 | GPIO_Pin_14 | GPIO_Pin_13
	#define _FOOT_PORT GPIOC
#elif defined  (__IOC_V2__)
	#define _12Voff_PIN GPIO_Pin_6
	#define _12Voff_PORT GPIOB
	#define _SYS_SHG_PIN 	GPIO_Pin_4
	#define _SYS_SHG_PORT GPIOB
	#define _SYS_SHG_sense_PIN 	GPIO_Pin_9
	#define _SYS_SHG_sense_PORT GPIOD
	#define _FOOT_MASK 	GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3
	#define _FOOT_PORT GPIOE
#else
#endif
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
