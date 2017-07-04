/**
	******************************************************************************
	* @file		spray.cpp
	* @author	Fotona d.d.
	* @version
	* @date		
	* @brief	Timers initialization & ISR
	*
	*/

/** @addtogroup
* @{
*/
#include	<stdio.h>
#include	<stdlib.h>
#include	<algorithm>
#include	"spray.h"
#include	"lcd.h"
#include	"ioc.h"
/*******************************************************************************
* Function Name	:
* Description		: 
* Output				:
* Return				: None
*******************************************************************************/
_SPRAY::_SPRAY() {	
					Water=			new _VALVE(7,true);
					Air=				new _VALVE(6,true);
					BottleIn=		new _VALVE(5,true);
					BottleOut=	new _VALVE(4,false);
	
					offset.air=offset.bottle=offset.compressor=	_BAR(1);
					gain.air=																		_BAR(2);
					gain.bottle=																_BAR(1.3);
					gain.compressor=														_BAR(1);
	
					Air_P=Bottle_P=0;
					AirLevel=WaterLevel=0;
					Bottle_ref=Air_ref=													_BAR(1);
	
					mode.On=false;
					vibrate=false;
					idx=0;

					BottleIn->Close();
					BottleOut->Close();
					Air->Close();
					Water->Close();
	
#ifdef 		__SIMULATION__
	
					plot.Clear();
					
					plot.Add(&pComp,1.0,0.02, LCD_COLOR_YELLOW);
					plot.Add(&pBott,1.0,0.02, LCD_COLOR_GREY);
					plot.Add(&pAir,	1.0,0.02, LCD_COLOR_MAGENTA);

#ifdef	USE_LCD
			pBott=pComp=pAir=pAmb=1.0;
			P1=4.0;P0=1.0;
			simrate=0;
#endif

		//		plot.Add(&_ADC::Instance()->buf.compressor,_BAR(1),_BAR(1)*0.02, LCD_COLOR_GREEN);
		//		plot.Add(&_ADC::Instance()->buf.bottle,_BAR(1),_BAR(1)*0.02, LCD_COLOR_CYAN);
		//		plot.Add(&_ADC::Instance()->buf.air,_BAR(1),_BAR(1)*0.002, LCD_COLOR_MAGENTA);
#endif
}
/*******************************************************************************
* Function Name :
* Description       : 
* Output                :
* Return                : None
*******************************************************************************/
#define   _P_THRESHOLD  0x8000
#define		_A_THRESHOLD	0x2000

int				_SPRAY::Poll() {

int		e=_NOERR;

					if(AirLevel) {
						Bottle_P += (Bottle_ref - (int)buffer.bottle)/16;
						if(Bottle_P < -_P_THRESHOLD) {
							Bottle_P=0;
							BottleIn->Close();
							BottleOut->Open(150,750);
						}
						if(Bottle_P > _P_THRESHOLD) {
							Bottle_P=0;
							BottleIn->Open(50,750);
							BottleOut->Close();
						}
					} else {
							BottleIn->Close();
							BottleOut->Open();
					}
					
					Air_ref			= offset.air + AirLevel*gain.air/10;																				// AirLevel od 0-10
					Bottle_ref	= offset.bottle + AirLevel*gain.bottle*(100+4*WaterLevel)/100/10;		

					if(!BottleIn->Busy() && !BottleOut->Busy()) {

					}
					
					if(WaterLevel && mode.On)
						Water->Open();
					else
						Water->Close();	

					if(AirLevel && mode.On) {
						Air_P += (Air_ref-(int)buffer.air);
						Air_P=__max(0,__min(_A_THRESHOLD*_PWM_RATE, Air_P));
						if(vibrate && __time__ % 50 < 10)
							Air->Open();
						else
							Air->Set(Air_P/_A_THRESHOLD);
						
						if(abs(adf.compressor - 4*offset.compressor) > offset.compressor/2)
							e |= _sprayInPressure;
					}
					else
						Air->Close();
					
					
#ifdef __SIMULATION__
			if(Simulator()) {
#ifdef USE_LCD
				if(plot.Refresh())
					lcd.Grid();
#endif
			}
#endif			
					
					return e;
}
/*******************************************************************************/
/**
	* @brief	TIM3 IC2 ISR
	* @param	: None
	* @retval : None
	*/
void			_SPRAY::LoadSettings(FILE *f) {
char			c[128];
					fgets(c,sizeof(c),f);
					sscanf(c,"%hu,%hu,%hu,%hu",&offset.cooler,&offset.bottle,&offset.compressor,&offset.air);
					fgets(c,sizeof(c),f);
					sscanf(c,"%hu,%hu,%hu,%hu",&gain.cooler,&gain.bottle,&gain.compressor,&gain.air);
					fgets(c,sizeof(c),f);
					sscanf(c,"%d,%d",&AirLevel,&WaterLevel);
}
/*******************************************************************************/
/**
	* @brief	TIM3 IC2 ISR
	* @param	: None
	* @retval : None
	*/
void			_SPRAY::SaveSettings(FILE *f) {
					fprintf(f,"%5d,%5d,%5d,%5d                 /.. offset\r\n", offset.cooler, offset.bottle, offset.compressor, offset.air);
					fprintf(f,"%5d,%5d,%5d,%5d                 /.. gain\r\n", gain.cooler, gain.bottle, gain.compressor, gain.air);
					fprintf(f,"%5d,%5d                             /.. air, H2O\r\n", AirLevel, WaterLevel);
}
/*******************************************************************************/
/**
	* @brief	TIM3 IC2 ISR
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
void			_SPRAY::Increment(int a, int b) {
#ifndef 	__SIMULATION__
					idx= __min(__max(idx+b,0),1);
#else
					idx= __min(__max(idx+b,0),3);
#endif			
					switch(idx) {
						case 0:
							AirLevel 		= __min(__max(0,AirLevel+a),10);
							break;
						case 1:
							WaterLevel 	= __min(__max(0,WaterLevel+a),10);
							break;
						case 2:
							P1 					= __min(__max(0.5,P1+(double)a/10.0),4.5);
							break;
						case 3:
							P0 					= __min(__max(0.5,P0+(double)a/10.0),1.5);
							break;
					}
#ifndef 	__SIMULATION__
					printf("\r:air/water   %3d,%3d,%3.1lf",
						AirLevel,WaterLevel,
							(double)(adf.compressor-offset.compressor)/gain.compressor);
					for(int i=1+4*(2-idx);i--;printf("\b"));	
#else
					printf("\r:air/water   %3d,%3d,%3.1lf,%3.1lf",
						AirLevel,WaterLevel,
							P1,P0);
					for(int i=1+4*(3-idx);i--;printf("\b"));	
#endif			
}
/*******************************************************************************
* Function Name	:
* Description		: 
* Output				:
* Return				: None T1,T2.T3   A0,A1,A2
*******************************************************************************/
#ifdef 		__SIMULATION__
bool			_SPRAY::Simulator(void) {
					
	_TIM		*tim=_TIM::Instance();

	#define Uc1 pComp
	#define Uc2 pBott
	#define Uc3 pAmb
	
	#define R1 100
	#define R2 100
	#define R4 300
	#define R7 300
	#define R6 10
	#define C1 1e-3
	#define C3 100e-6
	#define C2 50e-3
	#define dt 1e-3
	
	double	Iin=(P1-Uc1)/R1;
	double	I12=(Uc1-Uc2)/R2;
	double	I13=(Uc1-Uc3)/R7;
	double	I23=(Uc2-Uc3)/R4;
	double	I3=(Uc3-P0)/R6;
	double	Iout=(Uc2 - P0)/100.0;

	I13 = I13*tim->Pwm(6)/_PWM_RATE;

	if(BottleIn->Closed()) {
		I12=0;
		plot.Colour(&pBott,LCD_COLOR_GREY);
	} else
		plot.Colour(&pBott,LCD_COLOR_RED);
	
	if(BottleOut->Closed())
		Iout=0;
	else
		plot.Colour(&pBott,LCD_COLOR_BLUE);
		
	if(Water->Closed())
		I23=0;

	Uc1+=(Iin-I12-I13)/C1*dt;
	Uc2+=(I12-I23-Iout)/C2*dt;
	Uc3+=(I23+I13-I3)/C3*dt;	
	
	pAir=pAmb + I13*R7;
	buffer.compressor	=_BAR(pComp);
	buffer.bottle			=_BAR(pBott);
	buffer.air				=_BAR(pAir);
	
	buffer.V5		= _V5to16X;
	buffer.V12	= _V12to16X;
	buffer.V24	= _V24to16X;
	
	buffer.T2=(unsigned short)0xafff;
	
	if(simrate && __time__ < simrate)
		return false;
	
	simrate = __time__ + 10;
	return true;
}
#endif


