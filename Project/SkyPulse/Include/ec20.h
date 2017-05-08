#ifndef					EC20_H
#define					EC20_H
#include				"stm32f2xx.h"
#include				"stdio.h"
#include				"gpio.h"
#include				"isr.h"

typedef enum {									// ID's, uporabja se samo tiste s komentarjem...
	Sys2Ioc				=0x20,					//
	Ioc2Sys				=0x40,					//
	Sys2Ec				=0x21,					//
	Ec2Sys				=0x41,					//
	Ec2Sync				=0x42,					// sync. message, 300u prior to laser
	Can2ComIoc		=0xB0,					// IOC local console access req.
  Com2CanIoc		=0xB1,					// IOC local console data, transmit only, no filter
	Can2ComEc20		=0xBA,					// EC20 console access req. transmit only, no filter
	Com2CanEc20		=0xB3,					// EC20 console data			
	Sys2Ergm			=0x1f,
	Ergm2Sys			=0x3f,
	ErgmTrigger		=0x1a
} _stdid;

typedef enum {									// koda ukaza, 1.byte ...
	Id_EC20Status	=0x00,					// status report; ec >> sys 
	Id_EC20Cmd		=0x02,					// command frame; sys >> ec
	Id_EC20Set		=0x03,					// set Uo, To, mode; sys >> ec
	Id_EC20Reset	=0x14,					// set repetition, pw, fo; sys >> ec
	Id_EC20Eo			=0x07,					// energy ack; ec >> sys
} _code;

typedef enum {									// koda ukaza, 1.byte ...
	Id_ENGstest		=0xD100,				// selftest report
	Id_ENGset			=0xC100,				// set parameters
	Id_ENGget			=0xD101,				// get parameters
	Id_ENGdata		=0xD103,				// get parameters
} _wcode;

void			Send2Can(_stdid, void *, size_t);
//
//
// message objects with constructors and send method...
//
//
typedef __packed struct _EC20Status {
	_code						code;
	unsigned short	Status;
	unsigned short	Error;
	_EC20Status() : code(Id_EC20Status),Status(0),Error(0)			{}	
	void	Send(_stdid s) 																				{ Send2Can(s,(void *)&code,sizeof(_EC20Status)); };
	} _EC20Status;

typedef __packed struct _EC20Cmd {
	_code						code;
	unsigned short	Cmd;
	_EC20Cmd() : code(Id_EC20Cmd),Cmd(0) 												{}
	void	Send(_stdid s) 																				{ Send2Can(s,(void *)&code,sizeof(_EC20Cmd)); };
} _EC20Cmd;

typedef __packed struct _EC20Set {
	_code						code;
	unsigned short	Uo;
	unsigned short	To;
	unsigned char		Mode;
	_EC20Set() : code(Id_EC20Set),Uo(420),To(350),Mode(0x02)		{}
	void	Send(_stdid s)																				{ Send2Can(s,(void *)&code,sizeof(_EC20Set)); };
} _EC20Set;

typedef __packed struct _EC20Reset {
	_code						code;
	unsigned short	Period;
	unsigned short	Pw;
	unsigned char		Fo;
	_EC20Reset() : code(Id_EC20Reset),Period(2),Pw(300),Fo(100)	{}
	void	Send(_stdid s)																				{ Send2Can(s,(void *)&code,sizeof(_EC20Reset)); };
} _EC20Reset;

typedef __packed struct _EC20Eo {
	_code						code;
	unsigned short	UI;
	unsigned short	C;
	_EC20Eo() : code(Id_EC20Eo),UI(0),C(0) 											{}
	void	Send(_stdid s)																				{ Send2Can(s,(void *)&code,sizeof(_EC20Eo)); };
} _EC20Eo;

typedef __packed struct _ENGstest {
	_wcode						code;
	unsigned short 		EMST;
	_ENGstest() : code(Id_ENGstest),EMST(0)											{}
	void	Send(_stdid s)																				{ Send2Can(s,(void *)&code,sizeof(_ENGstest)); };
} _ENGstest;

typedef __packed struct _ENGset {
	_wcode						code;
	unsigned short 		EMNA,
										Emax,
										To;
	_ENGset() : code(Id_ENGset),EMNA(1),Emax(40000),To(1000)		{}
	void	Send(_stdid s)																				{ Send2Can(s,(void *)&code,sizeof(_ENGset)); };
} _ENGset;

typedef __packed struct _ENGget {
	_wcode						code;
	unsigned short 		EMST,
										Emax,
										To;
	_ENGget() : code(Id_ENGget),EMST(0),Emax(0),To(0)						{}
	void	Send(_stdid s)																				{ Send2Can(s,(void *)&code,sizeof(_ENGget)); };
} _ENGget;

typedef __packed struct _ENGdata {
	_wcode						code;
	unsigned short 		EMRE,
										Val1,
										Val2;
	_ENGdata() : code(Id_ENGdata),EMRE(0),Val1(0),Val2(0)				{}
	void	Send(_stdid s)																				{ Send2Can(s,(void *)&code,sizeof(_ENGdata)); };
} _ENGdata;

typedef __packed struct _ENGtrigger {
	_ENGtrigger()	{}
	void	Send()																								{ Send2Can(ErgmTrigger,NULL,0); };
} _ENGtrigger;


// ec20 command bits as per _EC20Cmd.Cmd parameter
//

#define		_HV1_EN			0x0001		// simmer req
#define 	_FOOT_REQ		0x0100		// footswitch req

// ec20 bit definition as from _EC20Status.Status parameter
//

#define 	_COMPLETED	0x8000		// selftest completed after startup or reboot
#define 	_SIM_DET		0x0001		// simmer ack
#define 	_FOOT_ACK		0x0200		// footswitch ack

#define		_STATUS_MASK				(_COMPLETED  +  _SIM_DET  +  _FOOT_ACK)


//
//
//
//
// EC20 object definition
//

class	_EC20 {
	private:
		void *parent;
		int		idx,timeout;
		short	biasPw,biasT,biasF,biasN,biasNo,bias_cnt;

		_EC20Status		EC20Status;
		_EC20Cmd			EC20Cmd;
		_EC20Set			EC20Set;
		_EC20Reset		EC20Reset;
		_EC20Eo				EC20Eo;
	
		_ENGstest			ENGstest;
		_ENGset       ENGset;
		_ENGget       ENGget;
		_ENGdata     	ENGdata;
		_ENGtrigger		ENGtrigger;	
	
	public:
		_EC20(void *);
		~_EC20();

	void		Parse(CanTxMsg	*);
	void		LoadSettings(FILE *);
	void		SaveSettings(FILE *);
	void		FootSwEvent(__FOOT);
	int			Increment(int, int);
	int			Refresh()													{ return Increment(0,0); };		
	
	int			IncrementBias(int, int);
	static 	void	ECsimulator(void *);

	bool		Timeout(void)											{ return timeout && __time__ > timeout; }
	void		Timeout(int t)										{ t > 0 ? timeout = __time__ + t : timeout=0; }
};
#endif
