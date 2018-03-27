#ifndef							IOC_H
#define							IOC_H

#include						<string.h>
#include						"can.h"

#define	_ADC_ERR_DELAY	200
#define _PUMP_ERR_DELAY	1000
#define _FAN_ERR_DELAY	3000
#define _EC20_EM_DELAY	5

typedef	enum {
	_NOERR						=0,
	_V5								=1<<0,
	_V12							=1<<1,
	_V24							=1<<2,
	_sprayInPressure	=1<<3,
	_sysOverheat			=1<<4,
	_pumpTacho				=1<<5,
	_pumpPressure			=1<<6,
	_pumpCurrent			=1<<7,
	_fanTacho					=1<<8,
	_emgDisabled			=1<<9,
	_handpcDisabled		=1<<10,
	_flowTacho				=1<<11,
	_energyMissing		=1<<12,
	_sprayNotReady		=1<<13,
	_doorswDisabled		=1<<14
} _Error;           
                 
typedef enum {    
	idIOC_State				=0x200,
	idIOC_SprayParm		=0x201,
	idIOC_Footreq			=0x202,
	idIOC_AuxReq			=0x203,
	idIOC_State_Ack		=0x240,
	idIOC_FootAck			=0x241,
	idIOC_SprayAck		=0x242,
	idIOC_AuxAck			=0x243,					// Tlsb Tmsb xx xx xx xx xx xx
	idCAN2COM					=0x20B,
  idCOM2CAN					=0x24B,
	idCAN2FOOT				=0x20C,
	idFOOT2CAN				=0x24C,
	idEC20_req				=0x280,
	idEM_ack					=0x0C0,
  idBOOT						=0x20
} _StdId;

typedef enum {
	_STANDBY,
	_READY,
	_ACTIVE,
	_ERROR		
} _State;

typedef enum {
	_OFF,
	_1,
	_2,
	_3,
	_4		
} _Footsw;

typedef enum {
	_SPRAY_NOT_READY,
	_SPRAY_READY,
	_VIBRATE
} _Spray;

typedef enum {					// gpioc	15 14 13 20 10
	__FOOT_OFF	=0xe000,	//				 1  1  1  x  x
	__FOOT_1		=0x2000,	//				 0  0  1  x  x
	__FOOT_2		=0xa000,	//				 1  0  1  x  x
	__FOOT_3		=0x8000,	//				 1  0  0  x  x
	__FOOT_4		=0xc000		//				 1  1  0  x  x
} __FOOT;

//_____________________________________________________________________
/*
IOC status report, send on:
	- request from SYS
	- on change of one of its members ...TBDF !!!
	- after report from ENG; laser loop control !!!
*/
typedef __packed struct _IOC_State {
	_State 	State;
	_Error	Error;	
	_IOC_State() : State(_STANDBY),Error(_NOERR)	{}
	void	Send() {
		CanTxMsg	m={idIOC_State_Ack,0,CAN_ID_STD,CAN_RTR_DATA,sizeof(_IOC_State),0,0,0,0,0,0,0,0};
		memcpy(m.Data,(const void *)&State,sizeof(_IOC_State));
		_CAN::Instance()->Send(&m);
	}
} _IOCstatus;
//_____________________________________________________________________
typedef __packed struct _IOC_FswAck {
	_Footsw State;
	_IOC_FswAck() : State(_OFF)	{}	
	void	Send() {
		CanTxMsg	m={idIOC_FootAck,0,CAN_ID_STD,CAN_RTR_DATA,sizeof(_IOC_FswAck),0,0,0,0,0,0,0,0};
		memcpy(m.Data,(const void *)&State,sizeof(_IOC_FswAck));
		_CAN::Instance()->Send(&m);
	}
} IOC_FootAck;
//_____________________________________________________________________


#endif
