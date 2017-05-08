#include	"app.h"
#include	"stm32f4_discovery_lcd.h"
#include	"stmpe811qtr.h"
/*******************************************************************************
* Function Name	: Timer_Init
* Description		: Configure timer pins as output open drain
* Output				 : TIM4
* Return				 : None
*******************************************************************************/
#define	PILE_PORT	GPIOB
#define PILE_BIT	GPIO_Pin_14
#define	PYRO_PORT	GPIOB
#define PYRO_BIT	GPIO_Pin_4
#define	notUSE_LCD
#define	__notPYRO__
int count=0;
#ifdef __PYRO__
int tpx[]={8570,80};
int tox[]={4710,80};
#else
int tpx[]={64390,200};
int tox[]={5500,90};
#endif
void	LcdDrawGrid() {
int	i;
		LCD_Clear(LCD_COLOR_BLACK);
		LCD_SetTextColor(0x1ce7);
		for(i=0; i<LCD_PIXEL_WIDTH; i+=50)
			LCD_DrawLine(i,0,LCD_PIXEL_WIDTH,LCD_DIR_VERTICAL);
		for(i=0; i<LCD_PIXEL_HEIGHT/2; i+=50) {
			LCD_DrawLine(0,LCD_PIXEL_HEIGHT/2+i,LCD_PIXEL_WIDTH,LCD_DIR_HORIZONTAL);
			LCD_DrawLine(0,LCD_PIXEL_HEIGHT/2-i,LCD_PIXEL_WIDTH,LCD_DIR_HORIZONTAL);
		}
		LCD_SetTextColor(LCD_COLOR_YELLOW);
		LCD_DrawLine(0,LCD_PIXEL_HEIGHT/2+100,LCD_PIXEL_WIDTH,LCD_DIR_HORIZONTAL);
		LCD_DrawLine(0,0,LCD_PIXEL_HEIGHT,LCD_DIR_VERTICAL);	
		
		LCD_SetTextColor(LCD_COLOR_YELLOW);
		for(i=0; i<6; ++i)
				LCD_DrawRect(4*LCD_PIXEL_WIDTH/5-10,
											i*LCD_PIXEL_HEIGHT/10+10,
											LCD_PIXEL_WIDTH/5,
											LCD_PIXEL_HEIGHT/10);
}

void 	Pile_Proc(char *p) {
int	 	t,n,data=0;
float	to,tp;
static int wrt=0,wstat=0,*q=NULL;
static _io *io=NULL;

		if(p) {
			int i;
			io=__stdin.IO;
			sscanf(p,"%d",&i);
			switch(i) {
				case 11:
				case 12:
				case 13:
				case 14:
				case 15:
					wrt=(i-10)*1000/16;
					count=LCD_PIXEL_WIDTH;
				break;
				case 17:
				case 18:
				case 19:
				case 20:
					wrt=(i-11)*1000/16;
					count=LCD_PIXEL_WIDTH;
				break;
			}
			if(i==21) {
				if(wrt)
					wrt=0;
				else
					wrt=-1;
			}
			
			if(i==23) {
				q=tox;
				printf("amb. temp.\r\n");
			}
			if(i==24) {
				q=tpx;
				printf("thermopile\r\n");
			}
			if(q && p[0]=='A') {
				q[0]+=10;
				printf("offset=%-5d\r\n",q[0]);
			}
			if(q && p[0]=='B') {
				if(q[0]>10) q[0]-=10;
				printf("offset=%-5d\r\n",q[0]);
			}
			if(q && p[0]=='C') {
				++q[1];
				printf("counts=%-5d\r\n",q[1]);
			}
			if(q && p[0]=='D') {
				if(q[1]>1) --q[1];
				printf("counts=%-5d\r\n",q[1]);
			}
		i=0;
		return;
		}
		
		switch(wstat++) {
			case 0:
				GPIO_ResetBits(PILE_PORT,PILE_BIT);											//low
				PILE_PORT->OTYPER &= ~PILE_BIT;													//PP
				for(t=0;t<20; ++t);
				GPIO_SetBits(PILE_PORT,PILE_BIT);												//high
				for(t=0;t<20; ++t);
				PILE_PORT->OTYPER |= PILE_BIT;													//OD
				break;
			case 2:
				wstat=0;
				break;
			case 1:		
#ifdef __PYRO__ 
				for(n=0;n<28;++n) {
#else
				for(n=0;n<31;++n) {
#endif
				GPIO_ResetBits(PILE_PORT,PILE_BIT);											//low
				PILE_PORT->OTYPER &= ~PILE_BIT;													//PP
				for(t=0;t<20; ++t);
				GPIO_SetBits(PILE_PORT,PILE_BIT);												//high
				for(t=0;t<20; ++t);
				PILE_PORT->OTYPER |= PILE_BIT;													//OD
				for(t=0;t<100; ++t);
				data <<= 1;		
				
				if(GPIO_ReadInputDataBit(PILE_PORT,PILE_BIT)==SET)	
					data |= 1;	
				}

				GPIO_ResetBits(PILE_PORT,PILE_BIT);											//low
				PILE_PORT->OTYPER &= ~PILE_BIT;													//PP
				for(t=0;t<20; ++t);
				GPIO_SetBits(PILE_PORT,PILE_BIT);												//high
				for(t=0;t<20; ++t);
				PILE_PORT->OTYPER |= PILE_BIT;													//OD
				for(t=0;t<100;++t);		

				to = (float)((data%0x4000)-tox[0])/(float)tox[1];	
				tp = (float)((data/0x4000)-tpx[0])/(float)tpx[1];	
				if(io && wrt) {
					io=_stdio(io);
					if(--wrt)
						printf("%d,%d,%5.1f,%5.1f\r\n",data % 0x4000,data/0x4000,to,tp);	//~~~
					io=_stdio(io);
				}
				
#ifdef	USE_LCD		
				{
static
int 			pix=0L;
uint8_t 	c[64];

					LCD_SetTextColor(LCD_COLOR_RED);
					sprintf((char *)c,"%5.1f",to);
					LCD_DisplayStringLine(0,c);
					LCD_SetTextColor(LCD_COLOR_GREEN);
					sprintf((char *)c,"%5.1f",tp);	//~~~
					LCD_DisplayStringLine(25,c);
					
					t=LCD_PIXEL_HEIGHT/2+100-tp; //~~~
					if(t==pix)                                                                                                                                                                                                   
						LCD_DrawLine(count,t,1,LCD_DIR_VERTICAL);		
					if(t>pix)
						LCD_DrawLine(count,pix,t-pix,LCD_DIR_VERTICAL);		
					if(t<pix)
						LCD_DrawLine(count,t,pix-t,LCD_DIR_VERTICAL);		
					pix=t;
					data=0;		
					
					LCD_SetTextColor(LCD_COLOR_RED);
					LCD_DrawLine(count,LCD_PIXEL_HEIGHT/2+100-to,1,LCD_DIR_HORIZONTAL);	
					
					if(count++ > LCD_PIXEL_WIDTH) {
						count=0;
						LcdDrawGrid();
					}
				}
#endif
			}
}



void	Pyro_Pyle_Proc(void *v) {
			Pile_Proc(NULL);			
}

void 	Pyro_Pile_Init() {
GPIO_InitTypeDef					GPIO_InitStructure;

//	IOE_Config();

		GPIO_StructInit(&GPIO_InitStructure);
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
		GPIO_InitStructure.GPIO_Pin = PYRO_BIT;
		GPIO_Init(PYRO_PORT, &GPIO_InitStructure);
		GPIO_SetBits(PYRO_PORT,PYRO_BIT);
	
		GPIO_StructInit(&GPIO_InitStructure);
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
		GPIO_InitStructure.GPIO_Pin = PILE_BIT;
		GPIO_Init(PILE_PORT, &GPIO_InitStructure);
		GPIO_SetBits(PILE_PORT,PILE_BIT);
		
#ifdef	USE_LCD
		STM32f4_Discovery_LCD_Init();
		LCD_SetBackColor(LCD_COLOR_BLACK);
		LCD_SetTextColor(LCD_COLOR_YELLOW);
		LCD_SetFont(&Font16x24);
		LcdDrawGrid();
#endif
		_thread_add(Pyro_Pyle_Proc,NULL,"pyro",1);
}


