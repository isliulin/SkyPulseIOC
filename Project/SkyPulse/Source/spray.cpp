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
					gain.air=																		_BAR(2.5);
					gain.bottle=																_BAR(1);
					gain.compressor=														_BAR(1);
	
					Air_P=Bottle_P=0;
					AirLevel=WaterLevel=0;
					Bottle_ref=Air_ref=													_BAR(1);
					Wgain=																			_BAR(0.5);											
	
					mode.Simulator=false;
					mode.Vibrate=false;
					mode.Water=false;
					mode.Air=false;
					idx=0;

					BottleIn->Close();
					BottleOut->Close();
					Air->Close();
					Water->Close();
					readyTimeout=0;
					offsetTimeout=__time__ + 5000;
					
					Pin=4.0;
					pComp= pBott=pNozz=Pout=1.0;
					simrate=0;
}
/*******************************************************************************
* Function Name :
* Description		: 
* Output				:
* Return				: None
*******************************************************************************/
#define   _P_THRESHOLD  0x8000
#define		_A_THRESHOLD	0x2000

int				_SPRAY::Poll() {
int				_err=_NOERR;
	
//------------------------------------------------------------------------------
					if(offsetTimeout && __time__ > offsetTimeout) {
						offset.bottle += adf.air - offset.air;
						offset.air = adf.air;
						offsetTimeout=0;
					}
//------------------------------------------------------------------------------
					Air_ref			= offset.air + AirLevel*gain.air/10;
					Bottle_ref	= offset.bottle + (Air_ref - offset.air)*gain.bottle/0x10000 + Wgain*WaterLevel/10;
//					Bottle_ref	= offset.bottle + AirLevel*gain.bottle*(100+4*WaterLevel)/100/10;
//------------------------------------------------------------------------------
					if(AirLevel || WaterLevel) {
						Bottle_P += (Bottle_ref - (int)buffer.bottle)/16;
						if(Bottle_P < -_P_THRESHOLD) {
							Bottle_P=0;
							BottleIn->Close();
							BottleOut->Open(150,750);
							if(readyTimeout)
								readyTimeout = __time__ + _SPRAY_READY_T;
						}
						if(Bottle_P > _P_THRESHOLD) {
							Bottle_P=0;
							BottleIn->Open(150,750);
							BottleOut->Close();
							if(readyTimeout)
								readyTimeout = __time__ + _SPRAY_READY_T;
						}
					} else {
							BottleIn->Close();
							BottleOut->Open();
					}

					if(10*(adf.compressor-offset.compressor)/gain.compressor < 25)
						_err |= _sprayInPressure;		
					if(readyTimeout && __time__ < readyTimeout)
						_err |= _sprayNotReady;
					else
						readyTimeout=0;

					if(mode.Water)
						Water->Open();
					else
						Water->Close();	

					if(AirLevel && mode.Air) {
						Air_P += (Air_ref-(int)buffer.air);
						Air_P=__max(0,__min(_A_THRESHOLD*_PWM_RATE, Air_P));
						if(mode.Vibrate && __time__ % 50 < 10)
							Air->Open();
						else
							Air->Set(Air_P/_A_THRESHOLD);						
					}
					else
						Air->Close();

					if(mode.Simulator && Simulator()) {
#ifdef USE_LCD
						if(lcd && plot.Refresh())
							lcd->Grid();
#endif
					}
					return _err;
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
}
/*******************************************************************************/
/**
	* @brief	TIM3 IC2 ISR
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
void			_SPRAY::Increment(int a, int b) {

					idx= __min(__max(idx+b,0),6);
					
					switch(idx) {
						case 0:
							AirLevel		= __min(__max(0,AirLevel+a),10);
							readyTimeout			= __time__ + _SPRAY_READY_T;
							break;
						case 1:
							WaterLevel	= __min(__max(0,WaterLevel+a),10);
							readyTimeout			= __time__ + _SPRAY_READY_T;
							break;
						case 2:
							break;
						case 3:
							gain.bottle	= __min(__max(_BAR(0.5),gain.bottle+100*a),_BAR(2));
							break;
						case 4:
							if(mode.Simulator) {
								Pout 				= __min(__max(0.5,Pout+(double)a/10.0),1.5);
								if(a) {
									AirLevel = WaterLevel;
									mode.Air = mode.Water = false;
									offsetTimeout = __time__ + 3000;
								}
							}
							break;
						case 5:
							if(a < 0)
								mode.Air=false;
							else if(a > 0)
								mode.Air=true;
							break;
						case 6:
							if(a < 0)
								mode.Water=false;
							else if(a > 0)
								mode.Water=true;
							break;
					}
					
					if(mode.Simulator) {
						printf("\r:spray %3d,%5d,%5.2lf,%5.2lf,%5.2lf",
							AirLevel,WaterLevel,
								(double)(adf.air-offset.air)/_BAR(1),
									(double)(adf.bottle-offset.bottle)/_BAR(1),
										Pout-1.0);
					} else {
						printf("\r:spray %3d,%5d,%5.2lf,%5.2lf,%5.2lf",
							AirLevel,WaterLevel,
								(double)(adf.air-offset.air)/_BAR(1),
									(double)(adf.bottle-offset.bottle)/_BAR(1),
										(double)(adf.compressor-offset.compressor)/_BAR(1));
					}
					if(mode.Air) 
						printf("   Air"); 
					else 
						printf("   ---"); 
					if(mode.Water) 
						printf(" Water"); 
					else 
						printf("   ---"); 
					
					for(int i=1+6*(6-idx); i--; printf("\b"));
}
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

bool			_SPRAY::Simulator(void) {
					
	_TIM		*tim=_TIM::Instance();

	#define Uc1		pComp
	#define Uc2		pBott
	#define Uc3		pNozz
	
	#define Rin		10
	#define Rout	100
	
	#define R2		100
	#define Rw		300
	#define Ra		300
	#define Rsp		100
	#define C1		1e-2
	#define C3		100e-6
	#define C2		50e-3
	#define dt		1e-3
	
	double	Iin=(Pin-Uc1)/Rin;
	double	I12=(Uc1-Uc2)/R2;
	double	I13=(Uc1-Uc3)/Ra;
	double	I23=(Uc2-Uc3)/Rw;
	double	I3=(Uc3-Pout)/Rsp;
	double	Iout=(Uc2 - Pout)/Rout;

	I13 = I13*tim->Pwm(6)/_PWM_RATE;

	if(BottleIn->Closed()) {
		I12=0;
		if(I23 < 0)
			plot.Colour(&_ADC::buffer.bottle,LCD_COLOR_GREEN);
		else
			plot.Colour(&_ADC::buffer.bottle,LCD_COLOR_GREY);
			
	} else
		plot.Colour(&_ADC::buffer.bottle,LCD_COLOR_RED);

	if(BottleOut->Closed())
		Iout=0;
	else
		plot.Colour(&_ADC::buffer.bottle,LCD_COLOR_BLUE);

	if(Water->Closed())
		I23=0;

	Uc1 += (Iin-I12-I13)/C1*dt;
	Uc2 += (I12-I23-Iout)/C2*dt;
	Uc3 += (I23+I13-I3)/C3*dt;	

	buffer.compressor	=_BAR(pComp);
	buffer.bottle			=_BAR(pBott + 0.05*I12*R2 + 0.03);
	buffer.air				=_BAR(pNozz + I13*Ra - 0.01);

	buffer.V5		= _V5to16X;
	buffer.V12	= _V12to16X;
	buffer.V24	= _V24to16X;

	buffer.T2=(unsigned short)0xafff;
	if(simrate && __time__ < simrate)
		return false;
	simrate = __time__ + 10;
	return true;
}



