#ifndef			TIM_H
#define			TIM_H
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
		bool	inv;
		_TIM *t;
	public:
		_VALVE(int k, bool i)					{	
																		t=_TIM::Instance(); 
																		n = k; 
																		inv=i;
																	};
		
		void Set(int i)								{ t->Pwm(n,i);										};
		void Set(int i, int j, int k)	{ t->Pwm(n,i,j,k);								};

		
		bool Opened(void)							{ if(inv) return t->Pwm(n) > _PWM_RATE/2; else return t->Pwm(n) < _PWM_RATE/2;};
		bool Closed(void)							{ if(inv) return t->Pwm(n) < _PWM_RATE/2; else return t->Pwm(n) > _PWM_RATE/2;};
		void Open(void)								{ inv ? Set(_PWM_RATE*9/10): Set(0);};
		void Close(void)							{ inv ? Set(0): Set(_PWM_RATE*9/10);};
		void Open(int i, int j)				{ inv ? Set(_PWM_RATE*9/10,i,j): Set(0,i,j);};
		void Close(int i, int j)			{ inv ? Set(0,i,j): Set(_PWM_RATE*9/10,i,j);};
		bool Busy(void)								{ return t->Busy(n);							};
};
//________________________________________________________________________________________________
class	_TIM3 {
	private:
		int to,timeout,tau[32],tauN;
	public:
		_TIM3(int);
		~_TIM3();
		static _TIM3 *Instance[];
		void	ISR(int);
		int		Tau(void);
};
//________________________________________________________________________________________________
class	_TIM9 {
	private:
		int to,timeout,tau[32],tauN;
	public:
		_TIM9(int);
		~_TIM9();
		static _TIM9 *Instance[];
		void	ISR(int);
		int		Tau(void);
};
#endif
