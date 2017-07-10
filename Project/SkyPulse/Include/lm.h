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
#include				<stdio.h>
#include				<stdlib.h>
#include				<ctype.h>
#include				"isr.h"
#include				"gpio.h"
#include				"term.h"
#include				"spray.h"
#include				"pump.h"
#include				"fan.h"
#include				"adc.h"
#include				"tim.h"
#include				"dac.h"
#include				"can.h"
#include				"ws2812.h"
#include				"ioc.h"

#define					SW_version	11

typedef enum		{DBG_CAN_TX, DBG_CAN_RX, DBG_ERR, DBG_INFO, DBG_CAN_COM=21, DBG_EC_SIM=22, DBG_ENRG=23}	_DEBUG_;
typedef enum		{PUMP, FAN, SPRAY, CTRL_A, CTRL_B, CTRL_C, CTRL_D, NONE} _SELECTED_;
//_____________________________________________________________________________
class	_LM {

	private:
		_SELECTED_ 	item;

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
		_TERM				console; 
		static int	debug, error_mask;
		static 			string ErrMsg[];
		double			plotA,plotB,plotC;
	
	_SPRAY				spray;
	_CAN					can;
	_PUMP					pump;
	_FAN					fan;
	_WS2812				ws;
	_IOC_State		IOC_State;
	_IOC_FootAck	IOC_FootAck;
	
		void 				Increment(int, int);
		void 				Select(_SELECTED_);
		void 				Submit(string);
		_SELECTED_	Active(void)			{	return item;	}
		
		void 				Refresh(void)			{	Increment(0,0);	}
		bool				Parse(FILE *),
								Parse(int);

		void				ErrParse(int);
		
		void				CanConsole(int, int);
		void				Foot2Can(void);
		
		static void	Poll(void *),
								Print(void *),
								Display(void *);
};
#endif
