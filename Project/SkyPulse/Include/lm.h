#ifndef					LM_H
#define					LM_H

/**
	******************************************************************************
	* @file		lm.cpp
	* @author	Fotona d.d.
	* @version
	* @date		
	* @class	_LM		
	* @brief	lightmaster application class
	*
	*/
#include				"stm32f2xx.h"
#include				<stdio.h>
#include				<stdlib.h>
#include				<ctype.h>
#include				"isr.h"
#include				"gpio.h"
#include				"term.h"
#include				"spray.h"
#include				"pump.h"
#include				"fan.h"
#include				"ec20.h"
#include				"ee.h"
#include				"adc.h"
#include				"tim.h"
#include				"dac.h"
#include				"can.h"
#include				"pyro.h"
#include				"pilot.h"
#include				"ws2812.h"

#define					SW_version	10

typedef enum		{DBG_CAN_TX, DBG_CAN_RX, DBG_ERR, DBG_INFO, DBG_CAN_COM=21, DBG_EC_SIM=22, DBG_ENRG=23}	_DEBUG_;
typedef enum		{PYRO, PYROnew, PILOT, PLOT_OFFSET, PLOT_SCALE, PUMP, FAN, SPRAY, 
									EC20 ,EC20bias, CTRL_A, CTRL_B, CTRL_C, CTRL_D, REMOTE_CONSOLE, NONE} _SELECTED_;
//_____________________________________________________________________________
class	_LM {

	private:
		_SELECTED_ 	item;
		_TERM				VT100; 

		int					Decode(char *c);
		int					DecodePlus(char *c);
		int					DecodeMinus(char *c);
		int					DecodeWhat(char *c);
		int					DecodeEq(char *c);
		int					errT;

	public:
		_LM();
		~_LM();
	
		_io					*io;
		static int	debug, error, error_mask;
		static 			string ErrMsg[];
		double			plotA,plotB,plotC;
	
		_PLOT<double> plot;	
		_SPRAY			spray;
		_CAN				can;
		_PYRO				pyro;
		_PUMP				pump;
		_FAN				fan;
		_EE					ee;
		_EC20				ec20;
		_PILOT			pilot;
		_WS2812			ws;

#ifdef	__DISCO__
		_LCD				lcd;
#endif

		void 				Increment(int, int);
		void 				Select(_SELECTED_);
		void 				Submit(string);
		_SELECTED_	Selected(void)		{	return item;	}
		
		void 				Refresh(void)			{	Increment(0,0);	}
		bool				Parse(FILE *);
		bool				Parse(void);
		
		bool				ErrTimeout(void)	{ return __time__ < errT; }
		void				ErrTimeout(int t)	{ errT = __time__ + t; }
		void				ErrParse(int);
		
		bool				Parse(int);
		void				RemoteConsole(int, int);
		
		static void	Poll(void *),
								Print(void *),
								Display(void *);
};
#endif
