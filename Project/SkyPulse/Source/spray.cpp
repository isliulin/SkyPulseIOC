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
					gain.air=																		_BAR(2.0);
					gain.bottle=																_BAR(0.5);											
					gain.compressor=														_BAR(1);
	
					Air_P=Bottle_P=0;
					AirLevel=WaterLevel=0;
					Bottle_ref=Air_ref=													_BAR(1);
					waterGain=																	_BAR(1.2);
	
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
					
					pFit=new _FIT();
					pFit->rp[0]=18049;
					pFit->rp[1]=11544*1e-4f;
					pFit->rp[2]=-1603*1e-8f;
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
					Bottle_ref	= offset.bottle + (Air_ref - offset.air)*pFit->Eval(Air_ref - offset.air)/0x10000 + gain.bottle*WaterLevel/10;
//				Bottle_ref	= offset.bottle + (Air_ref - offset.air)*waterGain/0x10000 + gain.bottle*WaterLevel/10;
//					Bottle_ref	= offset.bottle + AirLevel*waterGain*(100+4*WaterLevel)/100/10;
//------------------------------------------------------------------------------
					if(AirLevel || WaterLevel) {
						Bottle_P += (Bottle_ref - (int)adf.bottle)/16;
						if(Bottle_P < -_P_THRESHOLD) {
							Bottle_P=0;
							BottleIn->Close();
							BottleOut->Open(120,750);
							if(readyTimeout)
								readyTimeout = __time__ + _SPRAY_READY_T;
						}
						if(Bottle_P > _P_THRESHOLD) {
							Bottle_P=0;
							BottleIn->Open(120,750);
							BottleOut->Close();
							if(readyTimeout)
								readyTimeout = __time__ + _SPRAY_READY_T;
						}
					} else {
							BottleIn->Close();
							BottleOut->Open();
					}

					if(!(adf.compressor-offset.compressor)/gain.compressor)
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
						Air_P += (Air_ref-(int)adf.air);
						Air_P=__max(0,__min(_A_THRESHOLD*_PWM_RATE, Air_P));
						if(mode.Vibrate && __time__ % 50 < 10)
							Air->Open();
						else
							Air->Set(Air_P/_A_THRESHOLD);						
					}
					else
						Air->Close();
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
					fgets(c,sizeof(c),f);
					sscanf(c,"%d",&waterGain);
}
/*******************************************************************************/
/**
	* @brief	TIM3 IC2 ISR
	* @param	: None
	* @retval : None
	*/
void			_SPRAY::SaveSettings(FILE *f) {
					fprintf(f,"%5d,%5d,%5d,%5d                 /.. offset\r\n", offset.cooler, offset.bottle, offset.compressor, offset.air);
					fprintf(f,"%5d,%5d,%5d,%5d                 /.. gain\r\n", gain.cooler,gain.bottle,gain.compressor, gain.air);
					fprintf(f,"%5d                                   /..Wgain\r\n", waterGain);
}
/*******************************************************************************/
/**
	* @brief	TIM3 IC2 ISR
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
void			_SPRAY::Increment(int key) {
					_ADC::offset.air = _ADC::adf.air;
					_ADC::offset.bottle = _ADC::adf.bottle;
					printf("\r\n: air/water offset.... \r\n:");			
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
							gain.bottle	= __min(__max(_BAR(0.5),gain.bottle	+100*a),_BAR(2.5));
							break;
						case 4:
//							if(mode.Simulator) {
//								Pout 				= __min(__max(0.5,Pout+(double)a/10.0),1.5);
//								if(a) {
//									AirLevel = WaterLevel;
//									mode.Air = mode.Water = false;
//									offsetTimeout = __time__ + 3000;
//								}
//							}
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
					
//					if(mode.Simulator) {
//						printf("\r:spray %3d,%5d,%5.2lf,%5.2lf,%5.2lf",
//							AirLevel,WaterLevel,
//								(double)(adf.air-offset.air)/_BAR(1),
//									(double)(adf.bottle-offset.bottle)/_BAR(1),
//										Pout-1.0);
//					} else {
						printf("\r:spray %3d,%5d,%5.2lf,%5.2lf,%5.2lf",
							AirLevel,WaterLevel,
								(double)(adf.air-offset.air)/_BAR(1),
									(double)(adf.bottle-offset.bottle)/_BAR(1),
										(double)(adf.compressor-offset.compressor)/_BAR(1));
//					}
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



