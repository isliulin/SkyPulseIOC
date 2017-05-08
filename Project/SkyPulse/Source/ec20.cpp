/**
  ******************************************************************************
  * @file    dac.cpp
  * @author  Fotona d.d.
  * @version V1
  * @date    30-Sept-2013
  * @brief	 DA & DMA converters initialization
  *
  */

/** @addtogroup
* @{
*/

#include	"lm.h"
#include	"ec20.h"
#include	<math.h>
/*******************************************************************************/
/**
	* @brief	TIM3 IC2 ISR
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
_EC20::_EC20(void *v) {
	parent = v;
	biasPw=200;
	biasF=100;
	biasT=350;
	biasNo=biasN=0;
	
	idx = timeout = bias_cnt = 0;
}
/*******************************************************************************/
/**
	* @brief	TIM3 IC2 ISR
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
_EC20::~_EC20() {
}
/*******************************************************************************/
/**
	* @brief	TIM3 IC2 ISR
	* @param	: None
	* @retval : None
	*/
void		_EC20::LoadSettings(FILE *f) {
char		c[128];
				fgets(c,sizeof(c),f);
				sscanf(c,"%hu,%hu,%hu,%hu,%hu,%hu,%hu,%hu",&EC20Reset.Pw,&EC20Set.To,&EC20Reset.Period,&biasPw,&biasT,&biasF,&biasNo,&biasN);
}
/*******************************************************************************/
/**
	* @brief	Increment
	* @param	: None
	* @retval : None
	*/
/******************************************************************************/	
void		_EC20::SaveSettings(FILE *f) {
				fprintf(f,"%5d,%5d,%5d,%5d,%5d,%3d,%2d,%2d /.. EC20 settings\r\n",EC20Reset.Pw,EC20Set.To,EC20Reset.Period,biasPw,biasT,biasF,biasNo,biasN);
}
/*******************************************************************************/
/**
	* @brief	Increment
	* @param	: None
	* @retval : None
	*/
/******************************************************************************/
void		_EC20::FootSwEvent(__FOOT f) {
_LM 		*lm = static_cast<_LM *>(parent);

_EC20Set		set=EC20Set;
_EC20Reset	reset=EC20Reset;
_EC20Cmd		cmd=EC20Cmd;

				switch(f) {
					default:
					case __FOOT_OFF:
					case __FOOT_IDLE:
						lm->pilot.Off();
						lm->Submit("@standby.led");
						bias_cnt=biasNo+biasN;
						cmd.Cmd =0;
						cmd.Send(Sys2Ec);
						break;

					case __FOOT_MID:
						lm->pilot.On();
						lm->Submit("@ready.led");
						cmd.Cmd =_HV1_EN;
						cmd.Send(Sys2Ec);
						set.Uo = 10*EC20Set.Uo;
						bias_cnt=biasNo+biasN;
						if(bias_cnt--==0) {
							reset.Period=1000/EC20Reset.Period;
						} else {
							reset.Period=1000/biasF;
							reset.Pw=biasPw;
							set.To=biasT;
						}
						reset.Send(Sys2Ec);
						set.Send(Sys2Ec);
						break;

					case __FOOT_ACK:
						EC20Eo.C=0;
						set.Uo = 10*EC20Set.Uo;
						if(bias_cnt==0) {
							reset.Period=__max(1000/EC20Reset.Period - biasN*1000/biasF, 1000/biasF);
							reset.Send(Sys2Ec);
							set.Send(Sys2Ec);
							bias_cnt=biasN;
						} else if(bias_cnt--==biasN){
							reset.Period=1000/biasF;
							reset.Pw=biasPw;
							set.To=biasT;
							reset.Send(Sys2Ec);
							set.Send(Sys2Ec);
						}
						break;
						
 					case __FOOT_ON:
						EC20Eo.C=0;
						lm->Submit("@lase.led");
							cmd.Cmd =_HV1_EN + _FOOT_REQ;
							cmd.Send(Sys2Ec);
						break;
				}
}
/*******************************************************************************/
/**
	* @brief	Increment
	* @param	: None
	* @retval : None
	*/
/******************************************************************************/	
int				_EC20::Increment(int updown, int leftright) {
_LM 			*lm = static_cast<_LM *>(parent);		
char 			c[128];

					switch(idx=__min(__max(idx+leftright,0),3)) {
						case 0:			
							EC20Reset.Pw			= __min(__max(EC20Reset.Pw+updown,5),995);
						break;
						case 1: 
							EC20Set.To				= __min(__max(EC20Set.To+updown,100),1000);
						break;
						case 2:
							EC20Reset.Period	= __min(__max(EC20Reset.Period+updown,2),50);
						break;
						case 3:
							break;
					}
					sprintf(c,":EC20         %3.1lf%c,%4dus,%4dHz,",((double)EC20Reset.Pw)/10,'%', EC20Set.To, EC20Reset.Period);

					lm->pyro.Enabled=true;
					Timeout(EOF);
					switch(EC20Status.Status & _STATUS_MASK) {
						case _COMPLETED:																	// standby
							sprintf(strchr(c,'\0')," STNDBY");
							if(updown>0 && idx==3)
								FootSwEvent(__FOOT_MID);
						break;

						case _COMPLETED + _SIM_DET:												// simmer
							sprintf(strchr(c,'\0')," SIMMER");
							ENGset.To=EC20Set.To;
							ENGset.Send(Ec2Sys);
							if(updown>0 && idx==3)
								FootSwEvent(__FOOT_ON);
							if(updown<0 && idx==3)
								FootSwEvent(__FOOT_IDLE);
						break;

						case _COMPLETED  + _SIM_DET + _FOOT_ACK:
							sprintf(strchr(c,'\0')," LASE..");							// lasing
							if(updown<0 && idx==3)
								FootSwEvent(__FOOT_MID);
						break;

						default:
							lm->Submit("@standby.led");
							sprintf(strchr(c,'\0')," wait...");							// cakanje na ec20
							lm->pilot.Off();
							break;
						}
					
						if(EC20Eo.C) {
							if(ENGdata.Val1)
								sprintf(strchr(c,'\0'),"  %3.1lfJ,%5dW,%3.1lf'C,%3.1lf'C,%5.1lf",
																													(double)EC20Eo.C/1000,
																														EC20Eo.C*EC20Reset.Period/1000,
																															(double)_ADC::Th2o()/100,
																																(lm->plotA-7800.0)/200.0+25.0,
																																	(double)__max(0,(short)ENGdata.Val1)/10);
							else
								sprintf(strchr(c,'\0'),"  %3.1lfJ,%5dW,%3.1lf'C,%3.1lf'C, ---- ",
																													(double)EC20Eo.C/1000,
																														EC20Eo.C*EC20Reset.Period/1000,
																															(double)_ADC::Th2o()/100,
																																(lm->plotA-7800.0)/200.0+25.0);
																														
						}
													
						if(lm->Selected() == EC20) {
							printf("\r%s",c);
							int i=strlen(c)-(18+idx*7);
							while(--i)
							printf("\b");		
						}
						return 0;
}
/*******************************************************************************/
/**
	* @brief	Increment
	* @param	: None
	* @retval : None
	*/
/******************************************************************************/	
int				_EC20::IncrementBias(int updown, int leftright) {
_LM 			*lm = static_cast<_LM *>(parent);		
char 			c[128];

					switch(idx=__min(__max(idx+leftright,0),4)) {
						case 0:			
							biasPw						= __min(__max(biasPw+updown,5),995);
						break;
						case 1:			
							biasT							= __min(__max(biasT+updown,20),500);
						break;
						case 2:
							biasF							= __min(__max(biasF+updown,2),100);
						break;
						case 3:
							biasNo						= __min(__max(biasNo+updown,0),50);
							bias_cnt=biasNo+biasN;
						break;
						case 4:
							biasN							= __min(__max(biasN+updown,0),20);
							bias_cnt=biasNo+biasN;
						break;
					}

					sprintf(c,":EC20 bias    %3.1lf%c,%4dus,%4dHz,1st%3d,nxt%3d",((double)biasPw)/10,'%', biasT, biasF, biasNo, biasN);
								
					if(lm->Selected() == EC20bias) {
						printf("\r%s",c);
						int i=strlen(c)-(18+idx*7);
						while(--i)
						printf("\b");		
					}
					return 0;
}
/*******************************************************************************/
/**
	* @brief	Increment
	* @param	: None
	* @retval : None
	*/
/******************************************************************************/	
void				_EC20::Parse(CanTxMsg	*msg) {
_LM 				*lm = static_cast<_LM *>(parent);	

						switch(msg->StdId) {
/******************************************************************************/	
//____________Sys to EC20 message ______________________________________________
/******************************************************************************/	
							case Sys2Ec:
								switch(*msg->Data) {
//____________Sys to EC20 status req, only for debugging________________________________
									case Id_EC20Status:
										if(_BIT(_LM::debug, DBG_EC_SIM)) {
											EC20Status.Status= _COMPLETED;
											EC20Status.Send(Ec2Sys);
									}
									break;
//____________Sys to EC20 Uo, To, mode__________________________________________________
									case Id_EC20Set:
										memcpy(&EC20Set, msg->Data, msg->DLC);
										EC20Set.Uo /= 10;
									break;
//____________Sys to EC20 repetition, PW, fo ___________________________________________
									case Id_EC20Reset:
										memcpy(&EC20Reset, msg->Data, msg->DLC);
										EC20Reset.Period = 1000/EC20Reset.Period;
									break;
//____________Sys to EC20 command, only for debugging___________________________________
									case Id_EC20Cmd:
										memcpy(&EC20Cmd, msg->Data, msg->DLC);
										switch(EC20Cmd.Cmd) {
											case _HV1_EN:
												if(_BIT(_LM::debug, DBG_EC_SIM)) {
													EC20Status.Status=_COMPLETED + _SIM_DET;
													EC20Status.Send(Ec2Sys);
													_thread_remove((void *)ECsimulator,this);
												}
											break;
											case _HV1_EN + _FOOT_REQ:
												if(_BIT(_LM::debug, DBG_EC_SIM)) {
													EC20Status.Status=_COMPLETED  + _SIM_DET + _FOOT_ACK;
													EC20Status.Send(Ec2Sys);
													_thread_add((void *)ECsimulator,this,(char *)"EC20 simulator",1000/EC20Reset.Period);
												}
											break;
											default:
												if(_BIT(_LM::debug, DBG_EC_SIM)) {
													EC20Status.Status=_COMPLETED;
													EC20Status.Send(Ec2Sys);
												}
											break;
										}
									break;
									default:
										break;
								}
								break;
/******************************************************************************/	
//____________EC20 to Sys message ______________________________________________
/******************************************************************************/	
							case Ec2Sys:
								switch(*msg->Data) {
//____________EC20 to Sys status  ______________________________________________________
									case Id_EC20Status:
										memcpy(&EC20Status, msg->Data, msg->DLC);
										if(lm->Selected() == EC20)
											lm->Refresh();
									break;	
//____________EC20 to Sys energy  ______________________________________________________
									case Id_EC20Eo:
										lm->Submit("@energy.led");
										FootSwEvent(__FOOT_ACK);
										memcpy(&EC20Eo, msg->Data, msg->DLC);
										if(lm->pyro.Enabled && lm->Selected() == EC20)
											lm->Refresh();
									break;
								}
								break;
/******************************************************************************/	
//						Energymeter messages ______________________________________________
/******************************************************************************/	
//______________________________________________________________________________________
							case Ergm2Sys: 																						// energometer message 
								unsigned short *p=(unsigned short *)msg->Data;
								switch(*p) {
//____________ENERGOMETER SELFTEST  ______________________________________________________
									case Id_ENGstest:
										memcpy(&ENGstest, msg->Data, msg->DLC);
									break;	
//____________EC20 to Sys energy  ______________________________________________________
									case Id_ENGset:
										memcpy(&ENGset, msg->Data, msg->DLC);
									break;
//____________ENERGOMETER SELFTEST  ______________________________________________________
									case Id_ENGget:
										memcpy(&ENGget, msg->Data, msg->DLC);
									break;	
//____________ENERGOMETER SELFTEST  ______________________________________________________
									case Id_ENGdata:
										memcpy(&ENGdata, msg->Data, msg->DLC);		
											if(_BIT(_LM::debug, DBG_ENRG))
												printf(":%04d e1=%.1lf,e2=%.1lf\r\n>:",__time__ % 10000, 
													(double)__max(0,(short)p[2])/10,
														(double)__max(0,(short)p[3])/10);
										break;	
								}
								break;
						}
}


/*******************************************************************************/
/**
	* @brief	Increment
	* @param	: None
	* @retval : None
	*/
/******************************************************************************/	
void			Send2Can(_stdid std, void *v, size_t n) {
CanTxMsg	m={0,0,CAN_ID_STD,CAN_RTR_DATA,0,0,0,0,0,0,0,0,0};
					m.StdId=std;
					m.DLC=n;
					if(v)
						memcpy(m.Data,v,n);
					_CAN::Instance()->Send(&m);
}
/*******************************************************************************/
/**
	* @brief	Increment
	* @param	: None
	* @retval : None
	*/
/******************************************************************************/	
void		_EC20::ECsimulator(void *v) {
_EC20 *ec = static_cast<_EC20 *>(v);
				ec->EC20Eo.C=ec->EC20Eo.UI=pow((double)(ec->EC20Set.Uo * ec->EC20Reset.Pw / 1000),3)/420*ec->EC20Set.To/1000;
				ec->EC20Eo.Send(Ec2Sys);
}
/**
* @}
*/ 
