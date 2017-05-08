#ifndef					ADC_H
#define					ADC_H
#include				"stm32f2xx.h"

typedef struct	{
unsigned short	T2,T3,V5,V12,V24,cooler,bottle,compressor,air,Ipump;
} _ADMA;

__inline 
int			__fit(int to, const int t[], const int ft[]) {
int			f3=(ft[3]*(t[0]-to)-ft[0]*(t[3]-to)) / (t[0]-t[3]);
int			f2=(ft[2]*(t[0]-to)-ft[0]*(t[2]-to)) / (t[0]-t[2]);
int			f1=(ft[1]*(t[0]-to)-ft[0]*(t[1]-to)) / (t[0]-t[1]);
				f3=(f3*(t[1]-to) - f1*(t[3]-to)) / (t[1]-t[3]);
				f2=(f2*(t[1]-to)-f1*(t[2]-to)) / (t[1]-t[2]);
				return(f3*(t[2]-to)-f2*(t[3]-to)) / (t[2]-t[3]);
}

#define	_12Voff_ENABLE		GPIO_ResetBits(GPIOB,GPIO_Pin_3)
#define	_12Voff_DISABLE		GPIO_SetBits(GPIOB,GPIO_Pin_3)
#define	_SYS_SHG_ENABLE		GPIO_SetBits(GPIOB,GPIO_Pin_4)
#define	_SYS_SHG_ENABLED	GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_4)
#define	_SYS_SHG_DISABLE	GPIO_ResetBits(GPIOB,GPIO_Pin_4)

#define	_EMG_DISABLED			GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_8)

#define	_UREF							3.3
#define	_Rdiv(a,b)				((a)/(a+b))
#define	_Rpar(a,b)				((a)*(b)/(a+b))
#define	_V5to16X					(int)(5.0/_UREF*_Rdiv(820.0,820.0)*65535.0+0.5)			
#define	_V12to16X					(int)(12.0/_UREF*_Rdiv(820.0,3300.0)*65535.0+0.5)			
#define	_V24to16X					(int)(24.0/_UREF*_Rdiv(820.0,6800.0)*65535.0+0.5)			
	
#define	_16XtoV5(a)				(float)((float)a/65535.0*_UREF/_Rdiv(820.0,820.0))			
#define	_16XtoV12(a)			(float)((float)a/65535.0*_UREF/_Rdiv(820.0,3300.0))			
#define	_16XtoV24(a)			(float)((float)a/65535.0*_UREF/_Rdiv(820.0,6800.0))			

const int Ttab[]={ 1000, 2500, 5000, 8000 };
const	int Rtab[]={ (0xffff*_Rdiv(18813.0,5100.0)), (0xffff*_Rdiv(10000.0,5100.0)), (0xffff*_Rdiv(3894.6,5100.0)), (0xffff*_Rdiv(1462.6,5100.0))};

class	_ADC {
	private:
		int			n,timeout;
		static _ADC *instance;
	public:
		_ADC();
		static 	_ADMA	buffer,adf,offset,gain;
		static	int	Status(void);
		static	int	Th2o(void);
};

#endif
