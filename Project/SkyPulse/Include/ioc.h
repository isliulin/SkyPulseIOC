#ifndef							IOC_H
#define							IOC_H

#include						<string.h>
#include						"can.h"

#define	_ADC_ERR_DELAY	200
#define _PUMP_ERR_DELAY	3000
#define _FAN_ERR_DELAY	5000

typedef	enum {
	_NOERR						=0,
	_V5								=0x0001,
	_V12							=0x0002,
	_V24							=0x0004,
	_sprayInPressure	=0x0008,
	_sysOverheat			=0x0010,
	_pumpTacho				=0x0020,
	_pumpPressure			=0x0040,
	_pumpCurrent			=0x0080,
	_fanTacho					=0x0100,
	_emgDisabled			=0x0200,
	_pyroNoresp				=0x0400,
	_illstatereq			=0x0800,
} _Error;           
                  
typedef enum {    
	idIOC_State				=0x200,
	idIOC_SprayParm		=0x201,
	idIOC_SprayCmd		=0x202,
	idIOC_State_Ack		=0x240,
	idIOC_FootAck			=0x241,
	idIOC_SprayAck		=0x242,
	idCAN2COM					=0x20B,
  idCOM2CAN					=0x24B,
	idCAN2FOOT				=0x20C,
	idFOOT2CAN				=0x24C,
	idENGM						=0x0C0,
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
typedef __packed struct _IOC_FootAck {
	_Footsw State;
	_IOC_FootAck() : State(_OFF)	{}	
	void	Send() {
		CanTxMsg	m={idIOC_FootAck,0,CAN_ID_STD,CAN_RTR_DATA,sizeof(_IOC_FootAck),0,0,0,0,0,0,0,0};
		memcpy(m.Data,(const void *)&State,sizeof(_IOC_FootAck));
		_CAN::Instance()->Send(&m);
	}
} IOC_FootAck;
//_____________________________________________________________________
typedef __packed struct _IOC_SprayAck {
	_Spray	Status;
	_IOC_SprayAck() : Status(_SPRAY_NOT_READY)	{}	
	void	Send() {
		CanTxMsg	m={idIOC_SprayAck,0,CAN_ID_STD,CAN_RTR_DATA,sizeof(_IOC_SprayAck),0,0,0,0,0,0,0,0};
		memcpy(m.Data,(const void *)&Status,sizeof(_IOC_SprayAck));
		_CAN::Instance()->Send(&m);
	}
} IOC_SprayAck;
//_____________________________________________________________________


#endif
