/**
	******************************************************************************
	* @file		leds.cpp
	* @author	Fotona d.d.
	* @version
	* @date
	* @brief	WS2812B driver class
	*
	*/
	
/** @addtogroup
* @{
*/

#include	"ws2812.h"
#include	"isr.h"
#include	"term.h"
#include 	<ctype.h>
#include 	<string.h>
#include	<math.h>

ws2812 _WS2812::ws[] = 
			{{8,{0,0,0},NULL,noCOMM,NULL},
			{24,{0,0,0},NULL,noCOMM,NULL},
			{8,{0,0,0},NULL,noCOMM,NULL},
			{8,{0,0,0},NULL,noCOMM,NULL},
			{24,{0,0,0},NULL,noCOMM,NULL},
			{8,{0,0,0},NULL,noCOMM,NULL},
			{0,{0,0,0},NULL,noCOMM,NULL}};
/*******************************************************************************/
/**
	* @brief	_WS2812 class constructor
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
_WS2812::~_WS2812() {
			_thread_remove((void *)proc_WS2812,this);
			delete dma_buffer;
			for(int i=0; ws[i].size; ++i)
				delete ws[i].cbuf;
}
/*******************************************************************************/
/**
	* @brief	_WS2812 class constructor
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
_WS2812::_WS2812()  {
TIM_TimeBaseInitTypeDef		TIM_TimeBaseStructure;
TIM_OCInitTypeDef					TIM_OCInitStructure;
DMA_InitTypeDef						DMA_InitStructure;
GPIO_InitTypeDef					GPIO_InitStructure;
//
// ________________________________________________________________________________
			ws2812 *w=ws;
			int i=0;
			while(w->size)																		// count number of leds
				i+=w++->size;
			dma_buffer=new dma[i+1];													// allocate dma buffer
			dma_size=i*sizeof(dma)/sizeof(int)+1;
			
			w=ws;
			i=0;
			while(w->size) {
				w->cbuf=new HSV_set[w->size];										// alloc color buffer
				w->lbuf=&dma_buffer[i];													// pointer to dma tab
				i+=w++->size;
			}
			_thread_add((void *)proc_WS2812,this,(char *)"WS2812",10);
//
// ________________________________________________________________________________
// TIM2

			GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
			GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
			
			GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_TIM2);
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
			GPIO_Init(GPIOA, &GPIO_InitStructure);
	
// DMA setup _____________________________________________________________________
			DMA_StructInit(&DMA_InitStructure);
			RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
			DMA_DeInit(DMA1_Stream1);
			DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)dma_buffer;
			DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
			DMA_InitStructure.DMA_BufferSize = dma_size;
			DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
			DMA_InitStructure.DMA_Priority = DMA_Priority_High;
			DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
			DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
			DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
			DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
			DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;
			DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
			DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
			DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;

			DMA_InitStructure.DMA_Channel = DMA_Channel_3;
			DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)TIM2_BASE + 0x4C;	//~~~
			DMA_Init(DMA1_Stream1, &DMA_InitStructure);
// ________________________________________________________________________________
// TIMebase setup
			TIM_DeInit(TIM2);
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
			
			TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
			TIM_TimeBaseStructure.TIM_Prescaler = 0;
			TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
			TIM_TimeBaseStructure.TIM_Period = 74;

			TIM_TimeBaseInit(TIM2,&TIM_TimeBaseStructure);
// ________________________________________________________________________________
// Output Compare
			TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
			TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
			TIM_OCInitStructure.TIM_Pulse=0;
			TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
			TIM_OC3Init(TIM2, &TIM_OCInitStructure);
			TIM_OC3PreloadConfig(TIM2, TIM_OCPreload_Enable);
// ________________________________________________________________________________
// Startup
			TIM_CtrlPWMOutputs(TIM2, ENABLE);
			TIM_Cmd(TIM2,ENABLE);

			TIM_DMAConfig(TIM2, TIM_DMABase_CCR3, TIM_DMABurstLength_1Transfer);
			TIM_DMACmd(TIM2, TIM_DMA_Update, ENABLE);
		}
/*******************************************************************************/
/**
	* @brief	_WS2812 trigger method
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
void		_WS2812::trigger() {
int			i,j,k;
dma			*p;
RGB_set	q;
	
				for(i=0; ws[i].size; ++i)
					for(j=0; j<ws[i].size; ++j)
						if(ws[i].cbuf) {
							HSV2RGB(ws[i].cbuf[j], &q);
							for(k=0,p=ws[i].lbuf; k<8; ++k) {
								(q.b & (0x80>>k)) ? (p[j].b[k]=53)	: (p[j].b[k]=20);
								(q.g & (0x80>>k)) ? (p[j].g[k]=53)	: (p[j].g[k]=20);
								(q.r & (0x80>>k)) ? (p[j].r[k]=53)	: (p[j].r[k]=20);
							}
						}
						else
							for(k=0,p=ws[i].lbuf; k<24; ++k)
									p[j].g[k]=20;
				
				DMA_Cmd(DMA1_Stream1, DISABLE);
				TIM_Cmd(TIM2,DISABLE);
				TIM_SetCounter(TIM2,0);
				while(DMA_GetCmdStatus(DMA1_Stream1) != DISABLE);
				DMA_SetCurrDataCounter(DMA1_Stream1,dma_size);
				DMA_ClearFlag(DMA1_Stream1, DMA_FLAG_HTIF1 | DMA_FLAG_TEIF1 | DMA_FLAG_DMEIF1	| DMA_FLAG_FEIF1 | DMA_FLAG_TCIF1);
				DMA_Cmd(DMA1_Stream1, ENABLE);
				TIM_Cmd(TIM2,ENABLE);
}
/*******************************************************************************/
/**
	* @brief	_WS2812 class periodic task
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
void	 *_WS2812::proc_WS2812(_WS2812 *me) {
int			j,k,trg=0;
ws2812	*w=ws;
//------------------------------------------------------------------------------
				do {
					HSV_set	color = w->color;
					k=0;
//------------------------------------------------------------------------------
					switch(w->mode) {
						case noCOMM:
							break;
//------------------------------------------------------------------------------
						case FILL_OFF:
							color.v=0;
						case FILL_ON:
							for(j=k=0; j<w->size;++j) {
								w->cbuf[j].h = color.h;
								w->cbuf[j].s = color.s;

								if(w->cbuf[j].v < color.v)
									w->cbuf[j].v += (color.v - w->cbuf[j].v)/10+1;
								else if(w->cbuf[j].v > color.v)
									w->cbuf[j].v -= (w->cbuf[j].v - color.v)/10+1;
								else
									++k;
							}
						break;
//------------------------------------------------------------------------------
						case SWITCH_OFF:
							color.v=0;
						case SWITCH_ON:
							for(j=k=0; j<w->size;++j) {
								w->cbuf[j].h = color.h;
								w->cbuf[j].s = color.s;
								w->cbuf[j].v = color.v;
							}
							k=w->size;
						break;
//------------------------------------------------------------------------------
						case FILL_RIGHT_OFF:
							color.v=0;
						case FILL_RIGHT_ON:
							j=w->size; 
							k=0;
							while(--j) {
								w->cbuf[j].h = w->cbuf[j-1].h;
								w->cbuf[j].s = w->cbuf[j-1].s;
								
								if(w->cbuf[j].v < w->cbuf[j-1].v)
									w->cbuf[j].v += (w->cbuf[j-1].v - w->cbuf[j].v)/4+1;
								else if(w->cbuf[j].v > w->cbuf[j-1].v)
									w->cbuf[j].v -= (w->cbuf[j].v - w->cbuf[j-1].v)/4+1;
								else
									++k;
							}
							w->cbuf[j].h = color.h;
							w->cbuf[j].s = color.s;
							if(w->cbuf[j].v < color.v)
								w->cbuf[j].v += (color.v - w->cbuf[j].v)/4+1;
							else if(w->cbuf[j].v > color.v)
								w->cbuf[j].v -= (w->cbuf[j].v - color.v)/4+1;
							else
								++k;
							break;
//------------------------------------------------------------------------------
						case FILL_LEFT_OFF:
							color.v=0;
						case FILL_LEFT_ON:
							for(j=k=0; j<w->size-1;++j) {
								w->cbuf[j].h = w->cbuf[j+1].h;
								w->cbuf[j].s = w->cbuf[j+1].s;
								
								if(w->cbuf[j].v < w->cbuf[j+1].v)
									w->cbuf[j].v += (w->cbuf[j+1].v - w->cbuf[j].v)/4+1;
								else if(w->cbuf[j].v > w->cbuf[j+1].v)
									w->cbuf[j].v -= (w->cbuf[j].v - w->cbuf[j+1].v)/4+1;
								else
									++k;
							}
							w->cbuf[j].h = color.h;
							w->cbuf[j].s = color.s;
							if(w->cbuf[j].v < color.v)
								w->cbuf[j].v += (color.v - w->cbuf[j].v)/4+1;
							else if(w->cbuf[j].v > color.v)
								w->cbuf[j].v -= (w->cbuf[j].v - color.v)/4+1;
							else
								++k;
							break;
//------------------------------------------------------------------------------
						case RUN_RIGHT_OFF:
							color.v=0;
						case RUN_RIGHT_ON:
							for(j=k=0; j<w->size-1;++j) {
								if(w->cbuf[j] != w->cbuf[j+1])
									w->cbuf[j] = w->cbuf[j+1];
								else
									++k;
							}
							if(w->cbuf[j] != color)
								w->cbuf[j] = color;
							else
								++k;
							break;
//------------------------------------------------------------------------------
						case RUN_LEFT_OFF:
							color.v=0;
						case RUN_LEFT_ON:
							j=w->size; 
							k=0;
							while(--j) {
								if(w->cbuf[j] != w->cbuf[j-1])
									w->cbuf[j] = w->cbuf[j-1];
								else
									++k;
							}
							if(w->cbuf[j] != color)
								w->cbuf[j] = color;
							else
								++k;
							break;
//------------------------------------------------------------------------------
						default:
							break;
						}
//------------------------------------------------------------------------------					
						if(w->mode != noCOMM) {
							++trg;
							if(k==w->size)
								w->mode=noCOMM;
						}
				} while((++w)->size);

				if(trg)
					me->trigger();
				return NULL;
}
//______________________________________________________________________________________
int				strscan(char *s,char *ss[],int c) {
					int		i=0;
					while(1)
					{
						while(*s==' ') ++s;
						if(!*s)
							return(i);

						ss[i++]=s;
						while(*s && *s!=c)
						{
							if(*s==' ')
								*s='\0';
							s++;
						}
						if(!*s)
							return(i);
						*s++=0;
					}
}
//______________________________________________________________________________________
int				numscan(char *s,char *ss[],int c) {
					while(*s && !isdigit(*s)) 
						++s;
					return(strscan(s,ss,c));
}
/*******************************************************************************/
/**
	* @brief	_WS2812 parser, initial '.' character
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
int				_WS2812::ColorOn(char *c) {
char			*p=strtok(c," ,");
					switch(*p) {
//__________________________________________________
						case '0':
						case '1':
						case '2':
						case '3':
						case '4':
						case '5':
							do {
								ws[atoi(p)].mode=SWITCH_ON;
								p=strtok(NULL," ,");
								} while(p);
							break;
//__________________________________________________
						case 'f':
							for(p=strtok(NULL," ,"); p; p=strtok(NULL,","))
								ws[atoi(p)].mode=FILL_ON;
							break;
//__________________________________________________
						case 'l':
							for(p=strtok(NULL," ,"); p; p=strtok(NULL,","))
								ws[atoi(p)].mode=RUN_LEFT_ON;
							break;
//__________________________________________________
						case 'r':
							for(p=strtok(NULL," ,"); p; p=strtok(NULL,","))
								ws[atoi(p)].mode=RUN_RIGHT_ON;
							break;
//__________________________________________________

						default:
							return PARSE_MISSING;
					}
				return PARSE_OK;
				}	
/*******************************************************************************/
/**
	* @brief	_WS2812 parser, initial '.' character
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
int				_WS2812::ColorOff(char *c) {
char			*p=strtok(c," ,");
					switch(*p) {
//__________________________________________________
						case '0':
						case '1':
						case '2':
						case '3':
						case '4':
						case '5':
						do {
								ws[atoi(p)].mode=SWITCH_OFF;
								p=strtok(NULL," ,");
								} while(p);
							break;
//__________________________________________________
						case 'f':
							for(p=strtok(NULL," ,"); p; p=strtok(NULL,","))
								ws[atoi(p)].mode=FILL_OFF;
							break;
//__________________________________________________
						case 'l':
							for(p=strtok(NULL," ,"); p; p=strtok(NULL,","))
								ws[atoi(p)].mode=RUN_LEFT_OFF;
							break;
//__________________________________________________
						case 'r':
							for(p=strtok(NULL," ,"); p; p=strtok(NULL,","))
								ws[atoi(p)].mode=RUN_RIGHT_OFF;
							break;
//__________________________________________________
						default:
							return PARSE_MISSING;
					}
				return PARSE_OK;
				}	
/*******************************************************************************/
/**
	* @brief	_WS2812 class load/save settings method
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
int			_WS2812::SetColor(char *c) {
int			i;
				c=strtok(c,", ");
				switch(*c) {
					case '0':
					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
						i=atoi(c);
						ws[i].color.h =atoi(strtok(NULL,", "));
						ws[i].color.s =atoi(strtok(NULL,", "));
						ws[i].color.v =atoi(strtok(NULL,", "));
						break;						
					case 't':
						i=atoi(strtok(NULL,", "));
						if(i<5 || i>1000)
							return PARSE_ILLEGAL;
						_thread_find((void *)proc_WS2812,this)->dt=i;	
						break;
					default:
						return PARSE_ILLEGAL;
					}
				return PARSE_OK;
}
/*******************************************************************************/
/**
	* @brief	_WS2812 class load/save settings method
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
int		_WS2812::GetColor(int color) {
	
			_TERM key;
			ws2812 *w=&ws[color];
			int flag=0;
			
			printf("\n\rHSB:%d,%d,%d      ",w->color.h,w->color.s,w->color.v);
			while(1) {
				switch(key.Escape()) {
					case EOF:
						break;
					case __Up:
						++flag;
						w->color.h=__min(359,w->color.h + 1);
						break;				
					case __Down:
						++flag;
						w->color.h=__max(1,w->color.h - 1);
						break;
					case __Right:
						++flag;
						w->color.s=__min(255,w->color.s + 1);
						break;				
					case __Left:
						++flag;
						w->color.s=__max(1,w->color.s - 1);
						break;				
					case __PageUp:
						++flag;
						w->color.v=__min(255,w->color.v + 1);
						break;				
					case __PageDown:
						++flag;
						w->color.v=__max(1,w->color.v - 1);
						break;				
					case __Esc:
						return PARSE_OK;
				}
				if(flag) {
					printf("\rHSB:%d,%d,%d      ",w->color.h,w->color.s,w->color.v);
					for(int i=0; i<w->size; ++i)
						w->cbuf[i] =w->color;
					trigger();
					flag=0;
				}
				_wait(10,_thread_loop);
			}	
				}
/*******************************************************************************/
/**
	* @brief	_WS2812 class load/save settings method
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
void		_WS2812::SaveSettings(FILE *f){
				for(int i=0; ws[i].size; ++i)
					fprintf(f,"=color %d,%d,%d,%d\r\n",i,ws[i].color.h,ws[i].color.s,ws[i].color.v);
}
/*******************************************************************************
 * Function RGB2HSV
 * Description: Converts an RGB color value into its equivalen in the HSV color space.
 * Copyright 2010 by George Ruinelli
 * The code I used as a source is from http://www.cs.rit.edu/~ncs/color/t_convert.html
 * Parameters:
 *   1. struct with RGB color (source)
 *   2. pointer to struct HSV color (target)
 * Notes:
 *   - r, g, b values are from 0..255
 *   - h = [0,360], s = [0,255], v = [0,255]
 *   - NB: if s == 0, then h = 0 (undefined)
 ******************************************************************************/
void _WS2812::RGB2HSV(RGB_set RGB, HSV_set *HSV){
 unsigned char min, max, delta;
 
 if(RGB.r<RGB.g)min=RGB.r; else min=RGB.g;
 if(RGB.b<min)min=RGB.b;
 
 if(RGB.r>RGB.g)max=RGB.r; else max=RGB.g;
 if(RGB.b>max)max=RGB.b;
 
 HSV->v = max;                // v, 0..255
 
 delta = max - min;                      // 0..255, < v
 
 if( max != 0 )
 HSV->s = (int)(delta)*255 / max;        // s, 0..255
 else {
 // r = g = b = 0        // s = 0, v is undefined
 HSV->s = 0;
 HSV->h = 0;
 return;
 }
 
 if( RGB.r == max )
 HSV->h = (RGB.g - RGB.b)*60/delta;        // between yellow & magenta
 else if( RGB.g == max )
 HSV->h = 120 + (RGB.b - RGB.r)*60/delta;    // between cyan & yellow
 else
 HSV->h = 240 + (RGB.r - RGB.g)*60/delta;    // between magenta & cyan
 
 if( HSV->h < 0 )
 HSV->h += 360;
}
/*******************************************************************************
 * Function HSV2RGB
 * Description: Converts an HSV color value into its equivalen in the RGB color space.
 * Copyright 2010 by George Ruinelli
 * The code I used as a source is from http://www.cs.rit.edu/~ncs/color/t_convert.html
 * Parameters:
 *   1. struct with HSV color (source)
 *   2. pointer to struct RGB color (target)
 * Notes:
 *   - r, g, b values are from 0..255
 *   - h = [0,360], s = [0,255], v = [0,255]
 *   - NB: if s == 0, then h = 0 (undefined)
 ******************************************************************************/
void _WS2812::HSV2RGB(HSV_set HSV, RGB_set *RGB){
 int i;
 float f, p, q, t, h, s, v;
 
 h=(float)HSV.h;
 s=(float)HSV.s;
 v=(float)HSV.v;
 
 s /=255;
 
 if( s == 0 ) { // achromatic (grey)
 RGB->r = RGB->g = RGB->b = v;
 return;
 }
 
 h /= 60;            // sector 0 to 5
 i = floor( h );
 f = h - i;            // factorial part of h
 p = (unsigned char)(v * ( 1 - s ));
 q = (unsigned char)(v * ( 1 - s * f ));
 t = (unsigned char)(v * ( 1 - s * ( 1 - f ) ));
 
 switch( i ) {
 case 0:
 RGB->r = v;
 RGB->g = t;
 RGB->b = p;
 break;
 case 1:
 RGB->r = q;
 RGB->g = v;
 RGB->b = p;
 break;
 case 2:
 RGB->r = p;
 RGB->g = v;
 RGB->b = t;
 break;
 case 3:
 RGB->r = p;
 RGB->g = q;
 RGB->b = v;
 break;
 case 4:
 RGB->r = t;
 RGB->g = p;
 RGB->b = v;
 break;
 default:        // case 5:
 RGB->r = v;
 RGB->g = p;
 RGB->b = q;
 break;
 }
}

/**
* @}

.c 0,120,255,50 
.c 5,120,255,50 
.c 1,180,180,50
.c 4,180,180,50
.c 2,7,255,50
.c 3,7,255,50

.t 30
.c 2,7,255,50 
.c 3,7,255,50
.l 2,2
.r 3,2
.c 1,180,180,50 
.c 4,180,180,50
.r 1,2
.r 4,2
.d 3000
.c 2,7,255,0 
.c 3,7,255,0
.r 2,2
.l 3,2
.c 1,180,180,0 
.c 4,180,180,0
.l 1,2
.l 4,2
.d 3000

.c 2,7,255,50 
.c 3,7,255,50
.f 2,0
.f 3,0
.d 100
.c 2,7,255,0 
.c 3,7,255,0
.f 2,0
.f 3,0
.d 100
.c 2,7,255,50 
.c 3,7,255,50
.f 2,0
.f 3,0
.d 100
.c 2,7,255,0 
.c 3,7,255,0
.f 2,0
.f 3,0
.d 100
.c 2,7,255,50 
.c 3,7,255,50
.f 2,0
.f 3,0
.d 100
.c 2,7,255,0 
.c 3,7,255,0
.f 2,0
.f 3,0


.c 2,7,255,50 
.c 3,7,255,50
.f 2,2
.f 3,2
.c 1,180,180,50 
.c 4,180,180,50
.l 1,2
.l 4,2
.d 25
.c 1,180,180,0 
.c 4,180,180,0
.l 1,2
.l 4,2
.c 2,7,255,0 
.c 3,7,255,0
.f 2,2
.f 3,2

=color 0,180,180,95
=color 1,180,180,95
=color 2,7,255,95
=color 3,7,255,95
=color 4,180,180,95
=color 5,180,180,95

=color 0,180,180,50
=color 1,180,180,50
=color 2,7,255,50
=color 3,7,255,50
=color 4,180,180,50
=color 5,180,180,50

+c f,2
-c f,3
w 100
-c f,2
+c f,3
w 100
+c f,2
-c f,3
w 100
-c f,2
+c f,3
w 100
+c f,2
-c f,3
w 100
-c f,2
+c f,3
w 100
+c f,2
-c f,3
w 100
-c f,2
+c f,3
w 100
+c f,2
-c f,3
w 100
-c f,2
+c f,3
w 100
+c f,2
-c f,3
w 100
-c f,2
+c f,3
w 100
-c f,2
-c f,3

-c r,4
+c r,4

*/


