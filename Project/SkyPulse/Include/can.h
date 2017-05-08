#ifndef				CAN_H
#define				CAN_H
#include			"stm32f2xx.h"
#include			<stdio.h>
#include			"isr.h"
//______________________________________________________________________________________		
//
//
//
#ifdef	__DISCO__
#define				__CAN__						CAN1
#define				__FILT_BASE__			0
#define				CAN_GPIO					GPIOD
#define				CAN_RXPIN					0
#define				CAN_TXPIN					1
#define 			GPIO_AF_CAN 			GPIO_AF_CAN1	
#endif
#if defined   (__IOC_V1__) || defined  (__IOC_V2__)
#define				__CAN__						CAN2
#define				__FILT_BASE__			14
#define				CAN_GPIO					GPIOB
#define				CAN_RXPIN					5
#define				CAN_TXPIN					13
#define 			GPIO_AF_CAN 			GPIO_AF_CAN2	
#endif
//______________________________________________________________________________________		
//
//
//
class	_CAN {
	private:
		_io				*com, *oldcom;
		_io				*io;
	
	public:
#if defined (__DISCO__)
		_CAN	(bool=true);
#else
		_CAN	(bool=false);
#endif
		static _CAN *Instance(void);
	
		void	ISR_rx(void), ISR_tx(void);
		void 	Parse(void *);
		void	Send(CanTxMsg *);
		void	Send(char *);
		void	Recv(char *);
};

#endif

