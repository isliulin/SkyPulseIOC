#ifndef							IOC_H
#define							IOC_H

#include						<string.h>
#include						"can.h"




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
	_ec20noresp				=0x0800,
} _Error;           
                  
typedef enum {    
	idSYS2IOC_State		=0x200,
	idSYS2IOC_Spray		=0x201,
	idIOC2SYS_State		=0x240,
	idIOC2SYS_Footsw	=0x241,
	idIOC2SYS_Spray		=0x242,
	idCAN2COM					=0x0B0,
  idCOM2CAN					=0x0B1
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
	_SPRAY_READY
} _Spray;
//_____________________________________________________________________
/*
IOC status report, send on:
	- request from SYS
	- on change of one of its members ...TBDF !!!
	- after report from ENG; laser loop control !!!
*/
typedef __packed struct _IOC2SYS_State {
	_State 	State;
	_Error	Error;	
	_IOC2SYS_State() : State(_STANDBY),Error(_NOERR)	{}
	void	Send() {
		CanTxMsg	m={idIOC2SYS_State,0,CAN_ID_STD,CAN_RTR_DATA,sizeof(_IOC2SYS_State),0,0,0,0,0,0,0,0};
		memcpy(m.Data,(const void *)&State,sizeof(_IOC2SYS_State));
		_CAN::Instance()->Send(&m);
	}
} _IOCstatus;
//_____________________________________________________________________
typedef __packed struct _IOC2SYS_Footsw {
	_Footsw State;
	_IOC2SYS_Footsw() : State(_OFF)	{}	
	void	Send() {
		CanTxMsg	m={idIOC2SYS_Footsw,0,CAN_ID_STD,CAN_RTR_DATA,sizeof(_IOC2SYS_Footsw),0,0,0,0,0,0,0,0};
		memcpy(m.Data,(const void *)&State,sizeof(_IOC2SYS_Footsw));
		_CAN::Instance()->Send(&m);
	}
} _IOC2SYS_Footsw;
//_____________________________________________________________________
typedef __packed struct _IOC2SYS_Spray {
	_Spray	Spray;
	_IOC2SYS_Spray() : Spray(_SPRAY_NOT_READY)	{}	
	void	Send() {
		CanTxMsg	m={idIOC2SYS_Spray,0,CAN_ID_STD,CAN_RTR_DATA,sizeof(_IOC2SYS_Footsw),0,0,0,0,0,0,0,0};
		memcpy(m.Data,(const void *)&Spray,sizeof(_IOC2SYS_Spray));
		_CAN::Instance()->Send(&m);
	}
} _IOC2SYS_Spray;
//_____________________________________________________________________


#endif
