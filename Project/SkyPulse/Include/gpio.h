#ifndef					GPIO_H
#define					GPIO_H

#if defined   (__IOC_V0__)
	#define _12Voff_PIN GPIO_Pin_3
	#define _12Voff_PORT GPIOB
	#define _SYS_SHG_PIN 	GPIO_Pin_4
	#define _SYS_SHG_PORT GPIOB
	#define _cwbButton 	GPIO_Pin_8
	#define _cwbPort GPIOA
	#define _PILOT_PIN 	GPIO_Pin_13
	#define _FOOT_PORT GPIOC
	#define	_SYS_SHG_ENABLE		GPIO_SetBits(_SYS_SHG_PORT,_SYS_SHG_PIN)
	#define	_SYS_SHG_ENABLED	GPIO_ReadInputDataBit(_SYS_SHG_PORT,_SYS_SHG_PIN)
	#define	_SYS_SHG_DISABLE	GPIO_ResetBits(_SYS_SHG_PORT,_SYS_SHG_PIN)

#elif defined  (__IOC_V1__) || defined(__DISCO__) 
	#define _12Voff_PIN GPIO_Pin_3
	#define _12Voff_PORT GPIOB
	#define _SYS_SHG_PIN GPIO_Pin_4
	#define _SYS_SHG_PORT GPIOB
	#define _cwbButton 	GPIO_Pin_8
	#define _cwbPort GPIOA
	#define _PILOT_PIN 	GPIO_Pin_13
	#define _PILOT_PORT GPIOD
	#define	_SYS_SHG_ENABLE		GPIO_SetBits(_SYS_SHG_PORT,_SYS_SHG_PIN)
	#define	_SYS_SHG_ENABLED	GPIO_ReadInputDataBit(_SYS_SHG_PORT,_SYS_SHG_PIN)
	#define	_SYS_SHG_DISABLE	GPIO_ResetBits(_SYS_SHG_PORT,_SYS_SHG_PIN)

	#define _FSW_PORT		GPIOC
	#define _FSW0				GPIO_Pin_13
	#define _FSW1				GPIO_Pin_14
	#define _FSW2				GPIO_Pin_15

#elif defined  (__IOC_V2__)
	#define _12Voff_PIN GPIO_Pin_6
	#define _12Voff_PORT GPIOB
	#define _SYS_SHG_PIN 	GPIO_Pin_4
	#define _SYS_SHG_PORT GPIOB
	#define _cwbButton 		GPIO_Pin_9
	#define _cwbPort 			GPIOD
	#define _cwbDoor	 		GPIO_Pin_10
	#define _cwbHandpc		GPIO_Pin_11

	#define	_SYS_SHG_ENABLE		GPIO_ResetBits(_SYS_SHG_PORT,_SYS_SHG_PIN)
	#define	_SYS_SHG_ENABLED	!GPIO_ReadInputDataBit(_SYS_SHG_PORT,_SYS_SHG_PIN)
	#define	_SYS_SHG_DISABLE	GPIO_SetBits(_SYS_SHG_PORT,_SYS_SHG_PIN)

	#define _FSW_PORT		GPIOE
	#define _FSW0				GPIO_Pin_0
	#define _FSW1				GPIO_Pin_2
	#define _FSW2				GPIO_Pin_3
#else
#endif

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
