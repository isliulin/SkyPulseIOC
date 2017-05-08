#ifndef			TIM_H
#define			TIM_H

#include		"stm32f2xx.h"
#include		<stdio.h>
#define			_PWM_RATE			2000
#define			_PWon					_PWM_RATE*9/10	
#define			_PWoff				0	
//________________________________________________________________________________________________
class	_TIM {
	private:
		int t1[8],t2[8];
		_TIM();
	public:
		static	short speaker[];
		static	_TIM *Instance(void);
		void		Poll(void);
		bool		Busy(int);
		int			Pwm(int);
		int			Pwm(int, int);
		int			Pwm(int, int, int, int);
};
//________________________________________________________________________________________________
class	_VALVE {
	private:
		int 	n;
		_TIM *t;
	public:
		_VALVE(int k)									{	t=_TIM::Instance(); n = k;			};
		void Set(int i)								{ t->Pwm(n,i);										};
		void Set(int i, int j, int k)	{ t->Pwm(n,i,j,k);								};
		bool isOn(void)								{ return(t->Pwm(n)>_PWM_RATE/2);	};
		bool isOff(void)							{ return(t->Pwm(n)<_PWM_RATE/2);	};
		void On(void)									{ Set(_PWM_RATE*9/10);						};
		void Off(void)								{ Set(0);													};
		void On(int i, int j)					{ Set(_PWM_RATE*9/10,i,j);				};
		void Off(int i, int j)				{ Set(0,i,j);											};
		bool Busy(void)								{ return t->Busy(n);							};
		
		static short speaker[];
};

//________________________________________________________________________________________________
class	_TIM3 {
	private:
		int to,timeout,tau[32],tauN;
	public:
		_TIM3(int);
		~_TIM3();
		void	ISR(int);
		int		Tau(void);
};

#endif
