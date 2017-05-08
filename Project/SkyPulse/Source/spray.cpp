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
/*******************************************************************************
* Function Name	:
* Description		: 
* Output				:
* Return				: None
*******************************************************************************/
_SPRAY::_SPRAY() {	
					Bottle_ref=Air_ref=															_BAR(1);
					mode.On=false;
					AirLevel=WaterLevel=0;
					WaterLeft=300;
					WaterMax=450;
					WaterMin=150;
					vibrate=false;

					offset.air=offset.bottle=offset.compressor=	_BAR(1);
					gain.air=																		_BAR(2);
					gain.bottle=																_BAR(1.3);
					gain.compressor=														_BAR(1);	
	
					BottleOut=	new _VALVE(4);
					BottleIn=		new _VALVE(5);
					Air=				new _VALVE(6);
					Water=			new _VALVE(7);
					idx=0;

					BottleIn->Off();
					BottleOut->On();
					Air->Off();
					Water->Off();
					
#ifdef __SIMULATION__
					pComp=pAir=pBott=pAmb=1;
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

static int acc=0,accin=0,accout=0;
int		err=0;

					if(AirLevel) {
						Bottle_P += (Bottle_ref - (int)buffer.bottle)/16;
						if(Bottle_P < -_P_THRESHOLD) {
							Bottle_P=0;
							acc=adf.bottle;						
							accout = offset.bottle-adf.bottle;
							BottleIn->Off();
							BottleOut->Off(150,750);
						}
						if(Bottle_P > _P_THRESHOLD) {
							Bottle_P=0;
							acc=adf.bottle;	
							accin = adf.compressor-adf.bottle;
							BottleIn->On(50,750);
							BottleOut->On();
						}
					} else {
							BottleIn->Off();
							BottleOut->Off();
					}
					
					Air_ref			= offset.air + AirLevel*gain.air/10;
					Bottle_ref	= offset.bottle + AirLevel*gain.bottle*(100+4*WaterLevel)/100/10;
					

					if(!BottleIn->Busy() && !BottleOut->Busy()) {
						if(accin) {
//						printf("\r\n in  %d,%d",adf.bottle-acc,adf.bottle);
							WaterLeft += (adf.bottle-acc-WaterLeft)/4;
							acc=accin=0;
						}
						if(accout) {
//						printf("\r\n out %d,%d",adf.bottle-acc,adf.bottle);
							acc=accout=0;
						}		
					}
					
					if(WaterLevel && mode.On)
						Water->On();
					else
						Water->Off();	

					if(AirLevel && mode.On) {
						Air_P += (Air_ref-(int)buffer.air);
						Air_P=__max(0,__min(_A_THRESHOLD*_PWM_RATE, Air_P));
						if(vibrate && __time__ % 50 < 10)
							Air->On();
						else
							Air->Set(Air_P/_A_THRESHOLD);
						
						if(abs(adf.compressor - 4*offset.compressor) > offset.compressor/2)
							_SET_BIT(err,InputPressure);
					}
					else
						Air->Off();
					return err;
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
					idx= __min(__max(idx+b,0),1);
					switch(idx) {
						case 0:
							AirLevel 		= __min(__max(0,AirLevel+a),10);
							break;
						case 1:
							WaterLevel 	= __min(__max(0,WaterLevel+a),10);
							break;
						case 2:
							break;
						case 3:
							WaterMax += a;
							WaterMin += a;
							break;
					}
					printf("\r:air/water   %3d,%3d,%4.1lf",AirLevel,WaterLevel,(double)(adf.compressor-offset.compressor)/gain.compressor);
					for(int i=2+4*(2-idx);i--;printf("\b"));	
					
}
/*******************************************************************************
* Function Name	:
* Description		: 
* Output				:
* Return				: None T1,T2.T3   A0,A1,A2
*******************************************************************************/
#ifdef __SIMULATION__
#define C_comp 		100
#define	Rcomp			10

#define C_air 		5
#define	RairIn		100
#define	RairOut		5

#define C_bott 		1000
#define	RbottIn		5
#define	RbottOut	5

#define	Rh2o			1000

void			_SPRAY::Simulator() {
	_TIM		*tim=_TIM::Instance();
	
	double	iAir  =(pComp-pAir)/RairIn * tim->Pwm(6)/_PWM_RATE;
	double	oAir  =(pAir-pAmb)/RairOut;
	double	iBott = BottleIn->isOn() 	 ? (pComp-pBott)/RbottIn : 0;			//vpih
	double	oBott = BottleOut->isOff() ? (pBott-pAmb)/RbottOut : 0;			//izpuh
	double	iComp =(4.0-pComp)/Rcomp;
	double	oComp =iAir+iBott;

					if(Water->isOn())	{
						oBott += (pBott-pAir)/Rh2o;
						iAir += (pBott-pAir)/Rh2o;
					}
					
					pAir += (iAir-oAir)/C_air;
					pBott += (iBott-oBott)/C_bott;
					pComp += (iComp-oComp)/C_comp;
					
					buffer.compressor=pComp*_BAR(1);
					buffer.bottle=(pBott+iBott*RbottIn*0.1)*_BAR(1);
					buffer.air=(pAir + iAir*RairIn)*_BAR(1);
					
					buffer.V5		= _V5to16X;
					buffer.V12	= _V12to16X;
					buffer.V24	= _V24to16X;
						
					buffer.T2=(unsigned short)0xafff;
}
#endif


