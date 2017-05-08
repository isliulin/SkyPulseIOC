#include	"app.h"
#include "stm32f4_discovery_lcd.h"
/**
  ******************************************************************************
  * @file    appmisc.c
  * @author  Fotona d.d.
  * @version V1
  * @date    30-Sept-2013
  * @brief	 Application support 
  *
  */
/** @addtogroup PFM6_Application
* @{
*/
_buffer 	*_thread_buf=NULL;
//______________________________________________________________________________
_thread		*_thread_add(void *f,void *arg,char *name, int dt) {
_thread		*p=malloc(sizeof(_thread));
					if(p != NULL) {
						p->f=(func *)f;
						p->arg=arg;
						p->name=name;
						p->t=__time__;
						p->dt=dt;
						if(!_thread_buf)
							_thread_buf=_buffer_init(_THREAD_BUFFER_SIZE*sizeof(_thread *));
						_buffer_push(_thread_buf,&p,sizeof(_thread *));
					}
					return p;
}
//______________________________________________________________________________
void			_thread_loop(void) {
_thread		*p;
					if(_thread_buf)
						if(_buffer_pull(_thread_buf,&p,sizeof(_thread *))) {
							if(__time__ >= p->t) {
								p->to = __time__ - p->t;
								p->f(p->arg);
								p->t = __time__ + p->dt;
							}
							_buffer_push(_thread_buf,&p,sizeof(_thread *));
						}
}
//______________________________________________________________________________
void			_thread_remove(void  *f,void *arg) {
int				i=_buffer_count(_thread_buf)/sizeof(_thread *);
_thread		*p;
					while(i--) {
						_buffer_pull(_thread_buf,&p,sizeof(_thread *));
						if(f == p->f && arg == p->arg)
							free(p);
						else
							_buffer_push(_thread_buf,&p,sizeof(_thread *));
					}
}
//______________________________________________________________________________
_thread		*_thread_find(void  *f,void *arg) {
int				i=_buffer_count(_thread_buf)/sizeof(_thread *);
_thread		*p,*q=NULL;
					while(i--) {
						_buffer_pull(_thread_buf,&p,sizeof(_thread *));
						if(f == p->f && (arg == p->arg || !arg))
							q=p;
						_buffer_push(_thread_buf,&p,sizeof(_thread *));
					}
					return q;
}
//______________________________________________________________________________
void			_thread_list(void) {
int i			=_buffer_count(_thread_buf)/sizeof(_thread *);
_thread		*p;	
					printf("...\r\n");
					while(i--) {
						_buffer_pull(_thread_buf,&p,sizeof(_thread *));
						printf("%08X,%08X,%s,%d\r\n",(int)p->f,(int)p->arg,p->name,p->to);
						_buffer_push(_thread_buf,&p,sizeof(_thread *));
					}
}
//___________________________________________________________________________
void			_wait(int t,void (*f)(void)) {
int				to=__time__+t;
					while(to > __time__) {
						if(f)
							f();
					}
}
/*******************************************************************************
* Function Name : batch
* Description   :	ADP1047 output voltage setup, using the default format
* Input         :
* Output        :
* Return        :
*******************************************************************************/
int				batch(char *filename) {
FIL				f;
					if(f_open(&f,filename,FA_READ)==FR_OK) {
						__STDIN->file=&f;
						do {
							ParseCom(__STDIN);
						} while(!f_eof(&f));
						__STDIN->file=NULL;
						f_close(&f);
						return _PARSE_OK;
					} else
						return _PARSE_ERR_OPENFILE;
}
/*******************************************************************************
* Function Name : batch
* Description   :	ADP1047 output voltage setup, using the default format
* Input         :
* Output        :
* Return        :
*******************************************************************************/
#define maxx 40
#define maxy 20

typedef struct {
			char data[maxy][maxx];
			int	x,y;
			int	(*put)(_buffer *, int);
			_io	*io;
} lcd;

void	*refreshLCD(void *v) {
			int x,y;
lcd		*p=v;
sFONT *fnt = LCD_GetFont();
			for(y=0;y<maxy;++y)
				for(x=0;x<maxx;++x)
					LCD_DisplayChar(y*fnt->Height,x*fnt->Width,p->data[y][x]);
			_thread_find(refreshLCD,v)->t=__time__+5000;
			return v;
}

int		putLCD(_buffer *p, int c) {
			_thread	*t=_thread_find(refreshLCD,NULL);
			lcd *l;
			int x,y;
			
			if(p && t) {
				l=t->arg;
				if(l->put(p,c)==EOF)
					return EOF;
				switch(c) {
					case '\r':
						l->x=0;
						break;
					
					case '\n':
						if(l->y == maxy-1) {
							for(y=0;y<maxy-1;++y)
								for(x=0;x<maxx;++x)
									l->data[y][x]=l->data[y+1][x];
							for(x=0;x<maxx;++x)
								l->data[y][x]=' ';
						} else
							++l->y;			
						break;
						
					case '\b':
						if(l->x)
							--l->x;
						break;
							
					default:
						l->data[l->y][l->x++]=c;
						if(l->x == maxx)
							--l->x;
						break;
				}
				t->t=__time__+5;
			} else {
				if(!t) {
					l = malloc(sizeof(lcd));
					l->io=NULL;
				_thread_add(refreshLCD,l,"Lcd",20000);
				} else
					l=t->arg;
			
				if(l->io != __STDOUT) {
					if(l->io)
						l->io->put=l->put;
					l->io=__STDOUT;
					l->put=__STDOUT->put;
					__STDOUT->put=putLCD;
				}
				
				for(y=0;y<maxy;++y)
					for(x=0;x<maxx;++x)
						l->data[y][x]=' ';
				l->x=l->y=0;
				STM32f4_Discovery_LCD_Init();
				LCD_SetBackColor(LCD_COLOR_BLACK);
				LCD_SetTextColor(LCD_COLOR_YELLOW);
				LCD_SetFont(&Font8x12);
				
				if(c==EOF) {
					l->io->put=l->put;	
					_thread_remove(refreshLCD,l);
					free(l);
					l=NULL;
				}
			}
			return c;
}
//=============================================================================
//=  CRC32 generation                                                         =
//=============================================================================
#define POLYNOMIAL 0x04c11db7      									// Eth CRC-32 polynomial

int crc(int crc, int data) {
	int i=32;
	crc ^= data;
	while(i--)
		if(crc < 0)
			crc = (crc << 1) ^ POLYNOMIAL;
		else
			crc = (crc << 1);
		return crc;
}
//=============================================================================
//=  recursive wildcard compare, string s, * for string, ? for single character                                                     =
//=============================================================================
int wcard(char *t, char *s)
{
	return *t-'*' ? *s ? (*t=='?') | (toupper(*s)==toupper(*t)) && wcard(t+1,s+1) : 
		!*t : 
			wcard(t+1,s) || (*s && wcard(t,s+1));
}
//___________________________________________________________________________
void			PrintVersion(int v) {
//	int i=-1,
//			*p=(int *)__Vectors,
//			n=(FATFS_ADDRESS-(int)__Vectors)/sizeof(int);
//			while(n--)
//				i=crc(i,*p++);
	
					RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_CRC, ENABLE);
					CRC_ResetDR();
					printf(" %d.%02d %s, <%08X>+",
						v/100,v%100,
						__DATE__,
							CRC_CalcBlockCRC(__Vectors, (FATFS_ADDRESS-(int)__Vectors)/sizeof(int)));
}

/**
* @}
*/
			

