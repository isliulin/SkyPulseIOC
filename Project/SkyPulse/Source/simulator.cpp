/**
  ******************************************************************************
  * @file    lcd.cpp
  * @author  Fotona d.d.
  * @version V1
  * @date    30-Sept-2013
  * @brief	 DA & DMA converters initialization
  *
  */ 
/** @addtogroup
* @{
*/
#include	"simulator.h"
#include	"lm.h"

	#define Uc1		pComp
	#define Uc2		pBott
	#define Uc3		pNozz
	
	#define Rin		10
	#define Rout	100
	
	#define R2		100
	#define Rw		300
	#define Ra		300
	#define Rsp		100
	#define C1		0.01f
	#define C3		0.0001f
	#define C2		0.05f
	#define dt		0.001f
	
/*******************************************************************************
* Function Name	:
* Description		: 
* Output				:
* Return				: 
********************************************************************************

                         ///------R2----Uc2-----///-----Rout----Pout
                          |              |
                          |             ///
                          |              |
         Pin-----Rin-----Uc1              \____Rw_____
                          |                            \
                          |                            Uc3-----Rsp----Pout
                         XXX___________________Ra______/

*******************************************************************************/
_SIMULATOR::_SIMULATOR() {
	lcd=NULL;
	Pin=4.0f;	
	pComp= pBott= pNozz= Pout= 1.0f;
	
	plot.Clear();
	plot.Add(&_ADC::adf.compressor,_BAR(1.0),_BAR(0.02), LCD_COLOR_YELLOW);
	plot.Add(&_ADC::adf.bottle,_BAR(1.0),_BAR(0.02), LCD_COLOR_GREY);
	plot.Add(&_ADC::adf.air,_BAR(1.0),_BAR(0.02), LCD_COLOR_MAGENTA);
	
	rate=0;
	tau1=tau2=0;
	srand(__time__);
	
	offset.air += rand() % 1000 - 500;
	offset.bottle += rand() % 1000 - 500;
	offset.compressor += rand() % 1000 - 500;
}
/*******************************************************************************
* Function Name	:
* Description		: 
* Output				:
* Return				: 
********************************************************************************/
_SIMULATOR::~_SIMULATOR() {
	if(lcd) {
		LCD_Clear(LCD_COLOR_BLACK);
		delete lcd;
		lcd=NULL;
	}
	plot.Clear();
}
/*******************************************************************************
* Function Name	:
* Description		: 
* Output				:
* Return				: 
********************************************************************************/
void	_SIMULATOR::Poll(void *v) {
_LM		*lm = static_cast<_LM *>(v);
					
	_TIM		*tim=_TIM::Instance();
	
	float	Iin=(Pin-Uc1)/Rin;
	float	I12=(Uc1-Uc2)/R2;
	float	I13=(Uc1-Uc3)/Ra;
	float	I23=(Uc2-Uc3)/Rw;
	float	I3=(Uc3-Pout)/Rsp;
	float	Iout=(Uc2 - Pout)/Rout;

	I13 = I13*tim->Pwm(6)/_PWM_RATE;

	if(lm->spray.BottleIn->Closed()) {
		I12=0;
		if(I23 < 0)
			plot.Colour(&_ADC::adf.bottle,LCD_COLOR_GREEN);
		else
			plot.Colour(&_ADC::adf.bottle,LCD_COLOR_GREY);
			
	} else
		plot.Colour(&_ADC::adf.bottle,LCD_COLOR_RED);

	if(lm->spray.BottleOut->Closed())
		Iout=0;
	else
		plot.Colour(&_ADC::adf.bottle,LCD_COLOR_BLUE);

	if(lm->spray.Water->Closed())
		I23=0;

	Uc1 += (Iin-I12-I13)/C1*dt;
	Uc2 += (I12-I23-Iout)/C2*dt;
	Uc3 += (I23+I13-I3)/C3*dt;	

	adf.compressor	=_BAR(pComp);
	adf.bottle			=_BAR(pBott + 0.05f * I12*R2 + 0.03f);
	adf.air					=_BAR(pNozz + I13*Ra - 0.01f);

	adf.V5	= _V5to16X;
	adf.V12	= _V12to16X;
	adf.V24	= _V24to16X;

	adf.T2=(unsigned short)0xafff;
	
	if(__time__ > rate) {
		rate = __time__ + 10;
		if(lcd && plot.Refresh())
			lcd->Grid();
	}
	
	tau1 += ((float)_fTIM->CCR1 / _fTIM->ARR - tau1) / 1000.0f;
	lm->fan.Tau1 = tau1*10; 								//error po 10%

	if(lm->pump.Enabled) {
		tau2 += (DAC_GetDataOutputValue(0)/4096.0f - tau2) / 100.0f;
		adf.Ipump=tau2*(16.0*2.1*4094/3.3);     // 1:1
	}	else {
		tau2 += (0 - tau2) / 100.0f;
		adf.Ipump=0;
	}
	lm->fan.Tau2 = tau2*100;								// 50& = 500Hz 
}
/**
* @}
*/ 
