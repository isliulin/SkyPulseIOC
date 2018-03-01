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

#include "lm.h"
string _LM::ErrMsg[] = {
				"5V  supply",
				"12V supply",
				"24V supply",
				"spray input pressure",
				"cooler temperature",
				"pump rate out of range",
				"pump pressure out of range",
				"pump current out of range",
				"fan rate out of range",
				"emergency button pressed",
				"handpiece crowbar fail",
				"flow rate out of range",
				"energy report timeout",
				"spray not ready",
				"doorswitch crowbar fail"
};

int			_LM::debug=0,
				_LM::warn_mask	=	_sprayInPressure	+ _sprayNotReady + _flowTacho,
				_LM::error_mask	=	_sprayInPressure	+ _sprayNotReady + _flowTacho;

/*******************************************************************************/
/**
	* @brief	TIM3 IC2 ISR
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
_LM::_LM() {
			_thread_add((void *)Poll,this,(char *)"lm",1);
	
			FIL f;
			if(f_open(&f,"0:/lm.ini",FA_READ) == FR_OK) {

				pump.LoadSettings((FILE *)&f);
				fan.LoadSettings((FILE *)&f);
				spray.LoadSettings((FILE *)&f);

				while(!f_eof(&f))
					Parse((FILE *)&f);
				f_close(&f);	
			}	

			if(f_open(&f,"0:/limits.ini",FA_READ) == FR_OK) {
				pump.LoadLimits((FILE *)&f);
				fan.LoadLimits((FILE *)&f);
				f_close(&f);	
			}	

			_12Voff_ENABLE;
			_SYS_SHG_ENABLE;
			Parse(__F1);
			
      io=_stdio(NULL);
			Select(NONE);
			Submit("@onoff.led");
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
_LM::~_LM() {
			_thread_remove((void *)Poll,this);
			_thread_remove((void *)can.Console,this);
			_thread_remove((void *)ws.proc_WS2812,this);
}
/*******************************************************************************
* Function Name	:
* Description		:
* Output				:
* Return				:
*******************************************************************************/
//void	_LM::ErrParse(int e) {
//	
//			e &= ~error_mask;
//			e ? _RED1(3000): _GREEN1(20);
//			e = (e ^ IOC_State.Error) & e;
//			IOC_State.Error = (_Error)(IOC_State.Error | e);

//			if(e) {
//				_SYS_SHG_DISABLE;
//				IOC_State.State = _ERROR;
//				IOC_State.Send();
//				if(IOC_State.State != _ERROR)
//					Submit("@error.led");
//			}

//			for(int n=0; e && _BIT(_LM::debug, DBG_ERR); e >>= 1, ++n)
//				if(_BIT(e, 0))
//					printf("\r\nerror %03d: %s",n, ErrMsg[n].c_str());	
//}
/*******************************************************************************
* Function Name	:
* Description		:
* Output				:
* Return				:
*******************************************************************************/
void	_LM::ErrParse(int e) {
	
int		ee = (e ^ IOC_State.Error) & e & ~error_mask;
			if(ee && __time__ > 3000) {
				_SYS_SHG_DISABLE;
				if(IOC_State.State != _ERROR)
					Submit("@error.led");
				IOC_State.Error = (_Error)(IOC_State.Error | ee);
				IOC_State.State = _ERROR;
				IOC_State.Send();
			} 

int		ww=(e ^ IOC_State.Error) & warn_mask;
			if(ww && __time__ > 3000) {
				IOC_State.Error = (_Error)(IOC_State.Error ^ ww);
				IOC_State.Send();
			} 
			
			if(_SYS_SHG_ENABLED)
				_GREEN1(200);
			else
				_RED1(200);

			if(ee && _BIT(_LM::debug, DBG_ERR)) {
				for(int n=0; n<32; ++n)
					if(ee & (1<<n))
						printf("\r\nerror %03d: %s",n, ErrMsg[n].c_str());	
			} 	
}
/*******************************************************************************
* Function Name	:
* Description		:
* Output				:
* Return				:
*******************************************************************************/
void	_LM::Poll(void *v) {

_LM		*lm = static_cast<_LM *>(v);
_io		*temp=_stdio(lm->io);

int		err  = _ADC::Status();								// collecting error data
			err |= lm->pump.Poll();
			err |= lm->fan.Poll();
			err |= lm->spray.Poll();
			lm->ErrParse(err);										// parsing error data

			_TIM::Instance()->Poll();
			lm->can.Parse(lm);
			lm->Foot2Can();
			_stdio(temp);
}
/*******************************************************************************
* Function Name	:
* Description		:
* Output				:
* Return				:
*******************************************************************************/
extern _io *__com3;

void _LM::Foot2Can() {
			_io *temp=_stdio(__com3);
			CanTxMsg	tx={idFOOT2CAN,0,CAN_ID_STD,CAN_RTR_DATA,0,0,0,0,0,0,0,0,0};	
			while(tx.DLC  < 8) {
				int i=getchar();
				if(i==EOF)
					break; 
				else
				tx.Data[tx.DLC++] = i;
			}
			_stdio(temp);			
			if(tx.DLC > 0)
				can.Send(&tx);
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:	
* Return				:
*******************************************************************************/
void	_LM::Select(_ITEM i) {
			if(i != item)
				printf("\r\n:");
			item = i;
			Increment(0,0);
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:	
* Return				:
*******************************************************************************/
void	_LM::Increment(int i, int j) {
			switch(item) {
				case PUMP:
					pump.Increment(i,j);
					console.Refresh(200,__F5);
					break;
				
				case FAN:
					fan.Increment(i,j);
					console.Refresh(200,__F6);
					break;
				
				case SPRAY:
					spray.Increment(i,j);
					console.Refresh(200,__F7);
					break;
				
				case CTRL_A:
					pump.offset.cooler+=10*i;
					pump.gain.cooler+=10*j;
					printf("\r:cooler....... %5d,%5d",pump.offset.cooler,pump.gain.cooler);
					break;
				
				case CTRL_B:
					spray.offset.bottle+=10*i;
					spray.gain.bottle+=10*j;
					printf("\r:bottle....... %5d,%5d",spray.offset.bottle,spray.gain.bottle);
					break;

				case CTRL_C:
					spray.offset.compressor+=10*i;
					spray.gain.compressor+=10*j;
					printf("\r:compressor... %5d,%5d",spray.offset.compressor,spray.gain.compressor);
					break;
				
				case CTRL_D:
					spray.offset.air+=10*i;
					spray.gain.air+=10*j;
					printf("\r:air.......... %5d,%5d",spray.offset.air,spray.gain.air);
					break;
				
				default:
					break;
			}
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
int		_LM::DecodePlus(char *c) {
			switch(*c) {
				case 'd':
				case 'D':
					for(c=strchr(c,' '); c && *c;)
						_SET_BIT(debug,strtoul(++c,&c,10));
					break;
				case 'e':
				case 'E':
					for(c=strchr(c,' '); c && *c;)
						_CLEAR_BIT(error_mask,strtoul(++c,&c,10));
					break;
				case 'w':
				case 'W':
					for(c=strchr(c,' '); c && *c;)
						_SET_BIT(warn_mask,strtoul(++c,&c,10));
					break;
				case 'c':
					return ws.ColorOn(strchr(c,' '));
				case 'l':
				case 'L':
					ADC_DeInit();
#ifdef USE_LCD
					spray.plot.Clear();
					spray.plot.Add(&_ADC::adf.compressor,_BAR(1.0),_BAR(0.02), LCD_COLOR_YELLOW);
					spray.plot.Add(&_ADC::adf.bottle,_BAR(1.0),_BAR(0.02), LCD_COLOR_GREY);
					spray.plot.Add(&_ADC::adf.air,_BAR(1.0),_BAR(0.02), LCD_COLOR_MAGENTA);
					spray.lcd=new _LCD;
#endif
					spray.mode.Simulator=true;
					break;
				default:
					*c=0;
					return PARSE_SYNTAX;
			}
			*c=0;
			return PARSE_OK;
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
int		_LM::DecodeMinus(char *c) {
			switch(*c) {
				case 'd':
				case 'D':
					for(c=strchr(c,' '); c && *c;)
						_CLEAR_BIT(debug,strtoul(++c,&c,10));
					break;
				case 'e':
				case 'E':
					for(c=strchr(c,' '); c && *c;) {
						_SET_BIT(error_mask,strtoul(++c,&c,10));
						_CLEAR_BIT(IOC_State.Error,strtoul(c,&c,10));
					}
					break;
				case 'w':
				case 'W':
					for(c=strchr(c,' '); c && *c;) {
						_CLEAR_BIT(error_mask,strtoul(++c,&c,10));
					}
					break;
				case 'c':
					return ws.ColorOff(strchr(c,' '));
				case 'l':
				case 'L':
					new _ADC;
					spray.mode.Simulator=false;
					if(spray.lcd) {
						LCD_Clear(LCD_COLOR_BLACK);
						spray.lcd=NULL;
					}
					break;
				default:
					*c=0;
					return PARSE_SYNTAX;
			}
			*c=0;
			return PARSE_OK;
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
int		_LM::DecodeWhat(char *c) {
			switch(*c) {
				case 'p':
					_thread_list();
					break;
				case 'v':
					printf("\r\nV5=%4.1f,V12=%4.1f,V24=%4.1f",_16XtoV5(_ADC::adf.V5),_16XtoV12(_ADC::adf.V12),_16XtoV24(_ADC::adf.V24));			
					break;
				case 's':
					printf("\r\npI=%1.3lf,pB=%1.3lf,pA=%1.3lf,pC=%1.3lf",(double)_ADC::adf.compressor/_BAR(1),(double)_ADC::adf.bottle/_BAR(1),(double)_ADC::adf.air/_BAR(1),(double)_ADC::adf.cooler/_BAR(1));			
					break;
				case 'L':
					printf(",%08X",*(unsigned int *)strtoul(++c,&c,16));
					break;
				case 'W':
					printf(",%04X",*(unsigned short *)strtoul(++c,&c,16));
					break;
				case 'B':
					printf(",%02X",*(unsigned char *)strtoul(++c,&c,16));
					break;
				case 'd':
				case 'D':
					printf(" %0*X ",2*sizeof(debug)/sizeof(char),debug);
					break;
				case 'e':
				case 'E':
					for(int n=0,e=IOC_State.Error; e; e >>= 1, ++n)
						if(_BIT(e, 0))
							printf("\r\nerror %03d: %s",n, ErrMsg[n].c_str());	
						printf("\r\nerror mask=%08X\r\n:",error_mask);	
					break;
				case 'c':
					return ws.GetColor(atoi(strchr(c,' ')));
				case 'x':
				{
					RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_CRC, ENABLE);
					CRC_ResetDR();
					int i=EOF;
					while(*c) {
						int j=strtoul(++c,&c,16);
						i=crc(i,j);
						printf("\r\n%08X,%08X",CRC_CalcCRC(j),i);
						Watchdog();
					}
					printf("\r\n");
					break;
				}
				default:
					*c=0;
					return PARSE_SYNTAX;
			}
			*c=0;
			return PARSE_OK;
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
int		_LM::DecodeEq(char *c) {
			switch(*c) {
				case 'L': {
					int *cc=(int *)strtoul(++c,&c,16);
					while(*c)
						*cc++=(int)strtoul(++c,&c,16);
					}
					break;

				case 'W': {
					short *cc=(short *)strtoul(++c,&c,16);
					while(*c)
						*cc++=(short)strtoul(++c,&c,16);
					}
					break;

				case 'B': {
					char *cc=(char *)strtoul(++c,&c,16);
					while(*c)
						*cc++=(char)strtoul(++c,&c,16);
					}
					break;	

				case 'c':
					return ws.SetColor(strchr(c,' '));

				case 'k':
					break;
				
				case 'e':
				case 'E':
					while(*c)
						ErrParse(1 << strtoul(++c,&c,10));
					break;

				default:
					*c=0;
					return PARSE_SYNTAX;
			}
			*c=0;
			return PARSE_OK;
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
int		_LM::Decode(char *c) {
			switch(*c) {
				case 0:												// empty line						
				case ';':											// comment
					break;
				case '=':
					return DecodeEq(++c);
				case '?':
					return DecodeWhat(++c);
				case '+':
					return DecodePlus(++c);
				case '-':
					return DecodeMinus(++c);
				case 'v':
					PrintVersion(SW_version);
					break;
				case 'w':
					for(c=strchr(c,' '); c && *c;)
						_wait(strtoul(++c,NULL,0),_thread_loop);
					break;
				case '@':
					FIL f;
					if(f_open(&f,++c,FA_READ) != FR_OK)
						return PARSE_MISSING;
					while(!f_eof(&f))
						Parse((FILE *)&f);
					f_close(&f);	
					break;
				case '>':
					can.Send(++c);
					break;
				case '<':
					can.Recv(++c);
					break;

				default:
					*c=0;
					return PARSE_SYNTAX;					
			}
			*c=0;
			return PARSE_OK;
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
bool	_LM::Parse(FILE *f) {
_io		*io=_stdio(NULL);
bool	ret=Parse(fgetc(f));
			_stdio(io);
			return ret;
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
bool	_LM::Parse(int i) {
			switch(console.Esc(i)) {
				case EOF:
					break;
				case __F1:
				case __f1:
					printf("\r\n[F5]   - pump");
					printf("\r\n[F6]   - fan");
					printf("\r\n[F6]   - spray");
					printf("\r\n[F11]  - save settings");	
					printf("\r\n[F12]  - exit app.    ");	
					printf("\r\n");		
					printf("\r\nCtrlY  - reset");	
					printf("\r\nCtrlZ  - boot");	
					printf("\r\n:");	
					break;
				case __F2:
				case __f2:
					break;
				case __F3:
				case __f3:
					break;
				case __F4:
				case __f4:
					break;
				case __F5:
				case __f5:
					Select(PUMP);
					break;
				case __F6:
				case __f6:
					Select(FAN);
					break;			
				case __F7:
				case __f7:
					Select(SPRAY);
					break;
				case __F8:
				case __f8:
					break;
				case __F9:
				case __f9:
					break;				
				case __F10:
				case __f10:
					break;				
				case __F11:
				case __f11:
					FIL f;
					if(f_open(&f,"0:/lm.ini",FA_WRITE | FA_OPEN_ALWAYS) == FR_OK) {
						pump.SaveSettings((FILE *)&f);
						fan.SaveSettings((FILE *)&f);
						spray.SaveSettings((FILE *)&f);
						ws.SaveSettings((FILE *)&f);
						f_sync(&f);
						f_close(&f);							
						printf("\r\n saved...\r\n:");
					}	else				
						printf("\r\n file error...\r\n:");
					break;
				case __F12:
				case __f12:
				{
					printf("entering lib...\r\n>");
					_io*	io=_stdio(NULL);
					do {
						_thread_loop();
					} while(ParseCom(io));
					printf("...exit\r\n:");
					_stdio(io);
				}
				break;			
				case __Up:
					Increment(1, 0);
					break;				
				case __Down:
					Increment(-1, 0);
					break;	
				case __Left:
					Increment(0, -1);
					break;				
				case __Right:
					Increment(0, 1);
					break;
				case __CtrlA:
					Select(CTRL_A);
					break;
				case __CtrlB:
					Select(CTRL_B);
					break;
				case __CtrlC:
					Select(CTRL_C);
					break;
				case __CtrlD:
					Select(CTRL_D);
					break;
				case __CtrlE: 
					CanConsole(idCAN2COM,__CtrlE);
					break;	
				
				case __PageUp:
					if(item == SPRAY) {
						spray.waterGain=__min(spray.waterGain+1000,_BAR(2.5));
						Select(SPRAY);
					}
					break;
				case __PageDown:
					if(item == SPRAY) {
						spray.waterGain=__max(spray.waterGain-1000,_BAR(0.5));
						Select(SPRAY);
					}
					break;
				case __CtrlV:
					if(item == SPRAY) {
						if(spray.mode.Vibrate)
							spray.mode.Vibrate=false;
						else
							spray.mode.Vibrate=true;
				}
					break;
				case __Delete:
				case __CtrlI:
					if(item == SPRAY) {
						_ADC::offset.air = _ADC::adf.air;
						_ADC::offset.bottle = _ADC::adf.bottle;
						printf("\r\n: air/water offset.... \r\n:");
						Select(SPRAY);
					}
					break;
				case __CtrlR:
					if(item == SPRAY && spray.mode.Simulator) {
//						srand(__time__);
//						_ADC::offset.air += rand() % 1000 - 500;
//						_ADC::offset.bottle += rand() % 1000 - 500;
//						_ADC::offset.compressor += rand() % 1000 - 500;
//						printf("\r\n: offset randomized.... \r\n:");
//						Select(SPRAY);
					}
					break;
				case __CtrlQ:
					pump.Test();
					fan.Test();
					break;
				case __CtrlP:
					if(!pump.Align())
						printf("\r\n pump processing error...\r\n:");
					else if(!fan.Align()) 
						printf("\r\n fan processing error...\r\n:");
					else {
						FIL f;
						if(f_open(&f,"0:/limits.ini",FA_WRITE | FA_OPEN_ALWAYS) == FR_OK) {
							pump.SaveLimits((FILE *)&f);
							fan.SaveLimits((FILE *)&f);
							f_sync(&f);
							f_close(&f);							
							printf("\r\n saved...\r\n:");
						}	else				
							printf("\r\n file error...\r\n:");
					}
					break;
				case __FOOT_OFF:
					IOC_FootAck.State=_OFF;
					IOC_FootAck.Send();
					if(_BIT(_LM::debug, DBG_INFO))
						printf("\r\n:\r\n:footswitch disconnected \r\n:");					
					break;
				case __FOOT_1:
					IOC_FootAck.State=_1;
					IOC_FootAck.Send();
					if(_BIT(_LM::debug, DBG_INFO))
						printf("\r\n:\r\n:footswitch state 1\r\n:");					
					break;
				case __FOOT_2:
					IOC_FootAck.State=_2;
					IOC_FootAck.Send();
					if(_BIT(_LM::debug, DBG_INFO))
						printf("\r\n:\r\n:footswitch state 2\r\n:");					
					break;
				case __FOOT_3:
					IOC_FootAck.State=_3;
					IOC_FootAck.Send();
					if(_BIT(_LM::debug, DBG_INFO))
						printf("\r\n:\r\n:footswitch state 3\r\n:");					
					break;
				case __FOOT_4:
					IOC_FootAck.State=_4;
					IOC_FootAck.Send();
					if(_BIT(_LM::debug, DBG_INFO))
						printf("\r\n:\r\n:footswitch state 4\r\n:");					
					break;
				case __CtrlY:																	// reset call
					NVIC_SystemReset();
				case __CtrlZ:																	// bootloader call
					while(1);
				case __End:																		// exit call
					return false;
				default:
					if(console.Cmd(i) == NULL)
						break;
					if(int err=Decode(console.Cmd()))
						printf(" ...wtf(%02X)\r\n:",err);
					else
						printf("\r\n:");
					break;
			}
			return true;
}
/*******************************************************************************
* Function Name : batch
* Description   :	ADP1047 output voltage setup, using the default format
* Input         :
* Output        :
* Return        :
*******************************************************************************/
void	_LM::CanConsole(int k, int __ctrl) {
CanTxMsg	m={idCAN2COM,0,CAN_ID_STD,CAN_RTR_DATA,2,'v','\r',0,0,0,0,0};
int		j;
			printf(" remote console open... \r\n>");
			Select(NONE);																			// Select operation mode
			can.Send(&m);																			// send initial string
			do {																							// pull max. 8 characters from stdio
				for(m.DLC=0; m.DLC<8; ++m.DLC) {
					j=getchar();
					if(j == EOF || j == __ctrl)										// break if none or exit command
						break;
					m.Data[m.DLC]=j;															// else fill the can buffer
				}
				if(m.DLC > 0)																		// send if payload is there...
					can.Send(&m);
				_thread_loop();																	// call system loop
			} while (j != __ctrl);														// repeat until exit call
			m.DLC=0;																					// set empty payload
			can.Send(&m);																			// and send
			Select(NONE);																			// Close operation mode
			printf(" ...remote console closed\r\n>");
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void 	_LM::Submit(string str) {
			Decode((char *)str.c_str());
}
extern "C" {
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
int		lm() {
_LM 	lm;						
			while(1)	{
				_thread_loop();
_io*		io=_stdio(lm.io);
				if(lm.Parse(getchar()) == false)
					return 0;
				_stdio(io);
			} 

	}
}
