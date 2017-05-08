/**
  ******************************************************************************
  * @file    io.c
  * @author  Fotona d.d.
  * @version V1
  * @date    30-Sept-2013
  * @brief	 System I/O
  *
  */ 
	
/** @addtogroup PFM6_Misc
* @brief PFM6 miscellaneous
* @{
*/

#include		<stdlib.h>
#if defined  (STM32F2XX)
#include		"stm32f2xx.h"
#
#elif defined (__PVC__)
#include		"stm32f10x.h"
#elif	undefined (STM32F2XX || __PVC__)
*** undefined target !!!!
#endif

extern void			(*App_Loop)(void);
#if		defined (__PFM6__)
	#define	__LED_ON(a,b)			GPIO_ResetBits(a,b);
	#define	__LED_OFF(a,b)		GPIO_SetBits(a,b);
#elif  defined (__DISCO__)
	#define	__LED_OFF(a,b)		GPIO_ResetBits(a,b);
	#define	__LED_ON(a,b)			GPIO_SetBits(a,b);
#elif  defined (__PVC__)
	#define	__LED_OFF(a,b)
	#define	__LED_ON(a,b)
#else
	#### error, no HW defined
#endif


GPIO_TypeDef	*gpio[10];		// pointer na GPIO od indiv. leda
short					pin[10];			// oznaka pina 

/*******************************************************************************/
/**
  * @brief  LED processing
  * @param a: led ID, 0.9 as defined in pfm.h ... _RED1,_BLUE2 ... etc.
  * @param b: msecs, 0=off, -1=perm. on, a=-1 = periodic call, 1 msec
  * @retval : None
  */
void	_led(int a, int b) {
int				i;
static 	int 	t[]={1,1,1,1,1,1,1,1,1,1};

				if(a==-1) {
					for(i=0;i<10;++i) {
						if(t[i])
							if(!--t[i])
								_led(i,0);
					}
					return;
				}
				if(b==0) {
					t[a]=0;
					__LED_OFF(gpio[a], pin[a]);
				} else {
					if(b==-1)
						t[a]=0;
					else
						t[a]=b;
					__LED_ON(gpio[a], pin[a]);
				}
}
//______________________________________________________________________
#define	NN 200
#define	Nk 4
extern volatile int __time__;
//______________________________________________________________________
void		_lightshow() {
static 
void	(*f)(void)=NULL;
static	int	
				t1=0,
				t2=0,
				t3=0;
				
				if(f==NULL) {
					f=App_Loop;
					App_Loop=_lightshow;
				}
				if(__time__ < 10000) {
					if(!(++t1 % NN)) {
						_led(t3,0);
						t2=++t2 % Nk;
						if(t2==t3)
								_led((t3+1)%Nk,20);
					}
					if(!(t1 % ((NN*Nk)+1))) {
						t3=t2;
						_led(t3%Nk,20);
						_led((t3+1)%Nk,0);
					}			
				}
				f();
}
//______________________________________________________________________________________
//
// leds GPIO setup ______________________________________________________________
//
void	Initialize_LED(char *p[], int n) {
#if		defined (__PFM6__) || defined(__DISCO__)
GPIO_InitTypeDef	GPIO_InitStructure;
int		i;
			GPIO_StructInit(&GPIO_InitStructure);
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
#if		defined (__PFM6__)
			GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
#elif  defined (__DISCO__)
			GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
#else
	#### error, no HW defined
#endif
			if(!n)
				_lightshow();
			else
				for(i=0; i<n; ++i) {
					switch(*p[i]) {
						case 'a':gpio[i]=GPIOA;break;
						case 'b':gpio[i]=GPIOB;break;
						case 'c':gpio[i]=GPIOC;break;
						case 'd':gpio[i]=GPIOD;break;
						case 'e':gpio[i]=GPIOE;break;
						case 'f':gpio[i]=GPIOF;break;
						case 'g':gpio[i]=GPIOG;break;
						case 'h':gpio[i]=GPIOH;break;
						case 'i':gpio[i]=GPIOI;break;
						default:gpio[i]=NULL;
					}
					GPIO_InitStructure.GPIO_Pin = pin[i]=1<<(atoi(++p[i]));
					GPIO_Init(gpio[i], &GPIO_InitStructure);		
					__LED_OFF(gpio[i],pin[i]);
				}
#endif
}
