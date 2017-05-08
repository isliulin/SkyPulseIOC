/* Includes ------------------------------------------------------------------*/
#ifndef WIN32
#include				"stm32f2xx.h"
#endif
#include				<string.h>
#include				<stdio.h>
#include				<ctype.h>
#include				<math.h>
#include				"io.h"
#include				"ff.h"

#include 				"usbh_core.h"
#include 				"usbh_msc_usr.h"
#include 				"usbd_usr.h"
#include 				"usbd_desc.h"
#include 				"usbh_msc_core.h"
#include 				"usbd_msc_core.h"
#include 				"usbd_cdc_core.h"

#include 				"usb_conf.h"
#include 				"usbh_core.h"
#include				<stdint.h>

#define 				SW_version		0x100    

#define					_uS						60
#define					_mS						(1000*_uS)
void						_led(int, int),
								*_lightshow(void);

#define					_RED1(a)			_led(0,a)
#define					_GREEN1(a)		_led(1,a)
#define					_YELLOW1(a)		_led(2,a)
#define					_BLUE1(a)			_led(3,a)
#define					_ORANGE1(a)		_led(4,a)
#define					_RED2(a)			_led(5,a)
#define					_GREEN2(a)		_led(6,a)
#define					_YELLOW2(a)		_led(7,a)
#define					_BLUE2(a)			_led(8,a)
#define					_ORANGE2(a)		_led(9,a)

#define	 				__Esc					0x1b

#define					__CtrlA				0x01
#define					__CtrlB				0x02
#define					__CtrlC				0x03
#define					__CtrlD				0x04
#define					__CtrlE				0x05
#define					__CtrlF				0x06

#define					__CtrlI				0x09
#define					__CtrlO				0x0f
#define					__CtrlV				0x16
#define					__CtrlZ				0x1a
#define					__CtrlY				0x19

#define	 				__f1					0x001B4F50
#define	 				__f2					0x001B4F51
#define	 				__f3					0x001B4F52
#define	 				__f4					0x001B4F53
#define	 				__f5					0x001B4F54
#define	 				__f6					0x001B4F55
#define	 				__f7					0x001B4F56
#define	 				__f8					0x001B4F57
#define	 				__f9					0x001B4F58
#define	 				__f10					0x001B4F59
#define	 				__f11					0x001B4F5A
#define	 				__f12					0x001B4F5B

#define	 				__F1					0x5B31317E
#define	 				__F2 					0x5B31327E
#define	 				__F3					0x5B31337E
#define	 				__F4					0x5B31347E
#define	 				__F5					0x5B31357E
#define	 				__F6					0x5B31377E
#define	 				__F7					0x5B31387E
#define	 				__F8					0x5B31397E
#define	 				__F9					0x5B32307E
#define	 				__F10					0x5B32317E
#define	 				__F11					0x5B32337E
#define	 				__F12					0x5B32347E
#define	 				__Home				0x1B5B317E
#define	 				__End					0x1B5B347E
#define	 				__Insert			0x1B5B327E
#define	 				__PageUp			0x1B5B357E
#define	 				__Delete			0x1B5B337E
#define	 				__PageDown		0x1B5B367E
#define	 				__Up					0x001B5B41
#define	 				__Left				0x001B5B44
#define	 				__Down				0x001B5B42
#define	 				__Right				0x001B5B43

extern 
	volatile 
		int 				__debug__,
								__mode__,
								__status__,
								__error__,
								__event__;

#define					_DBG(a)				(__debug__ &  (1<<(a)))
#define					_SET_DBG(a)		(__debug__ |= (1<<(a)))
#define					_CLEAR_DBG(a)	(__debug__ &= ~(1<<(a)))

typedef					 enum
{								_DBG_CAN_TX,
								_DBG_CAN_RX,
								_DBG_ERR_MSG,
								_DBG_SYS_MSG
} 							___debug__;

								
#define					_EVENT(p,a)					(__event__ & (1<<(a)))
#define					_SET_EVENT(p,a)			 __event__ |= (1<<(a))
#define					_CLEAR_EVENT(p,a)		 __event__ &= ~(1<<(a))

#define					_ERROR(a)						(__error__ & (a))

#define					_CLEAR_ERROR(a)	do {																												\
									if(_DBG(_DBG_ERR_MSG) && (__error__ & (a))) {															\
_io 								*io=_stdio(__dbug);																												\
										printf(":%04d error %04X,%04X, clear\r\n>",__time__ % 10000,__error__,a);	\
										_stdio(io);																																\
									}																																						\
									__error__ &= ~(a);																														\
									} while(0)

#define					_SET_ERROR(a)	do {																													\
									if(_DBG(_DBG_ERR_MSG) && !(__error__ & (a))) {															\
_io 								*io=_stdio(__dbug);																												\
										printf(":%04d error %04X,%04X, set\r\n>",__time__ % 10000,__error__,a);		\
										_stdio(io);																																\
									}																																						\
									__error__ |= (a);																														\																																		\
								} while(0)

#define					_STATUS(a)				(__status__ & (a))
#define					_SET_STATUS(a)		(__status__ |= (a))
#define					_CLEAR_STATUS(a)	(__status__ &= ~(a))

#define					_MODE(a)					(__mode__ & (1<<(a)))
#define					_SET_MODE(a)			(__mode__ |= (1<<(a)))
#define					_CLEAR_MODE(a)		(__mode__ &= ~(1<<(a)))	
//________________________________________________________________________
#ifdef __DISCO__
#define					FATFS_START				FLASH_Sector_6
#define					FATFS_ADDRESS			0x8040000
#endif

#ifdef __PFM6__
#define					FATFS_START				FLASH_Sector_6
#define					FATFS_ADDRESS			0x8040000
#endif
//________________________________________________________________________            
extern int			(*USBH_App)(int);
int							USBH_Iap(int);        
//___________________________________________________________________________
void						deInitialize_usb(void),
								Initialize_host_msc(void),
								Initialize_device_msc(void),
								Initialize_device_vcp(void),
									
								TcpServerInit(void),
								USBD_Storage_Init(void);

#define					FSDRIVE_USB			"1:"
#define					FSDRIVE_CPU			"0:"

int							FLASH_Program(uint32_t, uint32_t); 
int							FLASH_Erase(uint32_t);

#define					__IWDGLOW__		4000
#define					__IWDGHIGH__	300
void						SysTick_init(void),
								Watchdog_init(int),
								WWDG_init(void),
										
								USBD_Vcp_Init(void),
								Initialize_LED(char *[], int),
								Initialize_NVIC(void);
int							batch(char *);
										
_io 						*Initialize_USART(int);

#define 	_THREAD_BUFFER_SIZE 128

typedef	void *func(void *);
 
typedef	struct {
func			*f;
void			*arg;
char			*name;
int				t,dt,to;
} _thread;

extern					_buffer 	*_thread_buf;
void						_thread_init(void),
								_thread_loop(void),
								_thread_list(void),
								_thread_remove(void *,void *);
_thread					*_thread_add(void *,void *,char *,int),
								*_thread_find(void *,void *);
								
void						_wait(int,void (*)(void));
								

extern _io			*__com0,
								*__dbug,
								*__can;
								
				
extern volatile int		__time__;
			
char						*cgets(int, int);
int							DecodeCom(char *),
								DecodeFs(char *);

_io 						*ParseCom(_io *);

void						USBHost(void);

__inline void		USBH_PowerOff(void) {
	#ifdef		__DISCO__
GPIO_InitTypeDef GPIO_InitStructure; 
								GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
								GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
								GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
								GPIO_Init(GPIOC, &GPIO_InitStructure);
								GPIO_SetBits(GPIOC,GPIO_Pin_0);
	#endif
}
__inline void		USBH_PowerOn(void) {
	#ifdef		__DISCO__
GPIO_InitTypeDef GPIO_InitStructure; 
								GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
								GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
								GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
								GPIO_Init(GPIOC, &GPIO_InitStructure);
								GPIO_ResetBits(GPIOC,GPIO_Pin_0);
	#endif
}
int							getHEX(char *, int);
void						putHEX(unsigned int,int);
int							hex2asc(int);
void						putHEX(unsigned int,int);
int							strscan(char *,char *[],int);
int							hex2asc(int);
void						putHEX(unsigned int,int);
int							sLoad(char *);
int							iDump(int *,int);
int							sDump(char *,int);
int							hex2asc(int);
int							asc2hex(int);
int 						wcard(char *, char *);
void						PrintVersion(int);
int							putLCD(_buffer *, int);

#ifndef	__max				
#define __max(a,b)  (((a) > (b)) ? (a) : (b))	
#endif
#ifndef	__min				
#define __min(a,b)  (((a) < (b)) ? (a) : (b))	
#endif

int							__fit(int,const int[],const int[]);
float						__lin2f(short);
short						__f2lin(float, short);
				        
extern					uint32_t	__Vectors[];

int8_t					STORAGE_Init (uint8_t);
int8_t					STORAGE_GetCapacity (uint8_t, uint32_t *, uint32_t *);
int8_t					STORAGE_IsReady (uint8_t);
int8_t					STORAGE_IsWriteProtected (uint8_t);
int8_t					STORAGE_Read (uint8_t, uint8_t *, uint32_t, uint16_t);
int8_t					STORAGE_Write (uint8_t, uint8_t *, uint32_t, uint16_t);
int8_t					STORAGE_GetMaxLun (void);


void						SectorQuery(void);
int 						Defragment(int);
				        				        
enum	err_parse	{
								_PARSE_OK=0,
								_PARSE_ERR_SYNTAX,
								_PARSE_ERR_ILLEGAL,
								_PARSE_ERR_MISSING,
								_PARSE_ERR_NORESP,
								_PARSE_ERR_OPENFILE,
								_PARSE_ERR_MEM
								};

__inline void dbg1(char *s) {
			if(_DBG(_DBG_SYS_MSG)) {
				_io *io=_stdio(__dbug);
				printf(":%04d %s\r\n>",__time__ % 10000, s);
				_stdio(io);
			}
}

__inline void dbg2(char *s, int arg1) {
			if(_DBG(_DBG_SYS_MSG)) {
				_io *io=_stdio(__dbug);
				printf(":%04d ",__time__ % 10000);
				printf(s,arg1);
				printf("\r\n>");
				_stdio(io);
			}
}

__inline void dbg3(char *s, int arg1, int arg2) {
			if(_DBG(_DBG_SYS_MSG)) {
				_io *io=_stdio(__dbug);
				printf(":%04d ",__time__ % 10000);
				printf(s,arg1,arg2);
				printf("\r\n>");
				_stdio(io);
			}
}
void			Pyro_Pile_Init(void),
					Pile_Proc(char *);


#define	GET_MACRO(_1,_2,_3,NAME,...) NAME
#define	_DEBUG_MSG(...) GET_MACRO(__VA_ARGS__, dbg3, dbg2, dbg1)(__VA_ARGS__)

#define ALL_GPIO	(RCC_AHB1Periph_GPIOA |RCC_AHB1Periph_GPIOB |RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE |RCC_AHB1Periph_GPIOF |RCC_AHB1Periph_GPIOG)
