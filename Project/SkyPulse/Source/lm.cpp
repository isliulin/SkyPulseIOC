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
				"spray air pressure too low",
				"cooler overheated",
				"pump tacho out of range",
				"pump pressure out of range",
				"pump current out of range",
				"fan tacho out of range",
				"emergency button pressed",
				"handpiece ejected",
				"EC20 not responding"
};
int			_LM::debug=0,
				_LM::error=0,
				_LM::error_mask=EOF;
/*******************************************************************************/
/**
	* @brief	TIM3 IC2 ISR
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
_LM::_LM() : ec20(this) {
			_thread_add((void *)Poll,this,(char *)"lm",1);
			_thread_add((void *)Display,this,(char *)"plot",1);			
	
			FIL f;
			if(f_open(&f,"0:/lm.ini",FA_READ) == FR_OK) {
				
// periph. settings
				pyro.LoadSettings((FILE *)&f);
				pump.LoadSettings((FILE *)&f);
				fan.LoadSettings((FILE *)&f);
				spray.LoadSettings((FILE *)&f);
				ec20.LoadSettings((FILE *)&f);
				pilot.LoadSettings((FILE *)&f);
				
// add. settings parsing
				while(!f_eof(&f))
					Parse((FILE *)&f);
				f_close(&f);	
			}	else				
				printf("\r\n setup file error...\r\n:");
			
			if(f_open(&f,"0:/limits.ini",FA_READ) == FR_OK) {
				pump.LoadLimits((FILE *)&f);
				fan.LoadLimits((FILE *)&f);
				f_close(&f);	
			}	else				
				printf("\r\n limits not active...\r\n:");
			
			if(f_open(&f,"0:/pw.ini",FA_READ) == FR_OK) {
				pyro.LoadFit((FILE *)&f);
				f_close(&f);	
			}	else				
				printf("\r\n fitting not active...\r\n:");

			printf("\r\n[F1]  - thermopile");
			printf("\r\n[F2]  - pilot");
			printf("\r\n[F4]  - spray on/off");
			printf("\r\n[F5]  - pump");
			printf("\r\n[F6]  - fan");
			printf("\r\n[F7]  - spray");
			printf("\r\n[F8]  - EC20 console");
			printf("\r\n[F11] - save settings");	
			printf("\r\n[F12] - exit app.    ");	
			printf("\r\n");	
			printf("\r\nCtrlE - EC20 console ");	
			printf("\r\nCtrlY - reset");	
			printf("\r\n:");	

			_12Voff_ENABLE;
			
// not used in the application
#ifdef	USE_LCD
	#ifdef	__SIMULATION__
			plot.Clear();
			plot.Add(&spray.pComp,1.0,0.02, LCD_COLOR_YELLOW);
			plot.Add(&spray.pBott,1.0,0.02, LCD_COLOR_CYAN);
			plot.Add(&spray.pAir,1.0,0.002, LCD_COLOR_MAGENTA);

//		plot.Add(&_ADC::Instance()->buf.compressor,_BAR(1),_BAR(1)*0.02, LCD_COLOR_GREEN);
//		plot.Add(&_ADC::Instance()->buf.bottle,_BAR(1),_BAR(1)*0.02, LCD_COLOR_CYAN);
//		plot.Add(&_ADC::Instance()->buf.air,_BAR(1),_BAR(1)*0.002, LCD_COLOR_MAGENTA);

	#endif
#endif
      io=_stdio(NULL);
			ErrTimeout(3000);
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
			_thread_remove((void *)Print,this);
			_thread_remove((void *)Display,this);
}
/*******************************************************************************
* Function Name	:
* Description		:
* Output				:
* Return				:
*******************************************************************************/
void	_LM::ErrParse(int e) {
	
			e &= error_mask;
			e ? _RED1(200): _GREEN1(20);
	
			if(ec20.Timeout()) {
				_SET_BIT(e,ec20noresp);
				ec20.Timeout(EOF);
			}

			if(e ^ _LM::error) {
				if(_BIT(e,pyroNoresp)) {
					pump.Disable();
					Submit("@ejected.led");
				} else {
					pump.Enable();
					Submit("@inserted.led");
					_CLEAR_BIT(_LM::error,pyroNoresp);
				}
			}

			e = (e ^ _LM::error) & e;									// extract the rising edge only 
			_LM::error |= e;													// OR into LM error register

			if(!ErrTimeout())	{
				if(e) {
					ErrTimeout(5000);
//					if(e & error_mask) {								// mask off inactive errors...
//						Submit("@error.led");
						_SYS_SHG_DISABLE;
//					}

				} else {
					_SYS_SHG_ENABLE;
					_LM::error=0;
				}
			}

			for(int n=0; e && _BIT(_LM::debug, DBG_ERR); e >>= 1, ++n)
				if(_BIT(e, 0))
					printf("\r\nerror %03d: %s",n, ErrMsg[n].c_str());	
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
			err |= lm->pyro.Error();
			lm->ErrParse(err);										// parsing error data
	
			lm->can.Parse(lm);										
			lm->pilot.Poll();
			_TIM::Instance()->Poll();


#ifdef __SIMULATION__
			lm->spray.Simulator();
#ifdef USE_LCD
//			if(!(++me->zzz % 10) && me->plot.Refresh())
//				me->lcd.Grid();
#endif
#endif			
			_stdio(temp);
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:	
* Return				:
*******************************************************************************/
void	_LM::Select(_SELECTED_ i) {
			if(i != item)
				printf("\r\n:");
			item = i;
			Refresh();
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
					if(i)
						ErrTimeout(1000);
					pump.Increment(i,j);
					break;
				
				case FAN:
					if(i)
						ErrTimeout(1000);
					fan.Increment(i,j);
					break;
				
				case SPRAY:
					spray.Increment(i,j);
					break;
				
				case EC20:
					ec20.Increment(i,j);
					break;

				case EC20bias:
					ec20.IncrementBias(i,j);
					break;
				
				case PILOT:
					pilot.Increment(i,j);
					break;
				
				case PYRO:
					if(i || j || pyro.Enabled) {
						pyro.Enabled=false;
						pyro.Increment(i,j);
					}	else {
						pyro.Enabled=true;
						printf("\r\n");
						plot.Clear();
						plot.Add(&plotA,0,1, LCD_COLOR_YELLOW);
						plot.Add(&plotB,2813,10, LCD_COLOR_CYAN);
						plot.Add(&plotC,0,1, LCD_COLOR_YELLOW);
					}
					break;

				case PYROnew:
					pyro.Enabled=true;
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
				
				case PLOT_OFFSET:
					plot.Offset(i,j);
					break;
				
				case PLOT_SCALE:
					plot.Scale(i,j);
					break;
				
				default:
					break;
			}
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				: _thread_add((void *)poll_callback,this,(char *)"lm",10);
*******************************************************************************/
int		_LM::DecodePlus(char *c) {
			switch(*c) {
				case 'P':
					_thread_add((void *)_LM::Print,this,(char *)"lm",strtoul(++c,NULL,0));
					break;
				case 'D':
					for(c=strchr(c,' '); c && *c;)
						_SET_BIT(debug,strtoul(++c,&c,10));
					break;
				case 'E':
					for(c=strchr(c,' '); c && *c;)
						_SET_BIT(error_mask,strtoul(++c,&c,10));
					break;
				case 'f':
					pyro.addFilter(++c);
					break;
				case 'c':
					return ws.ColorOn(strchr(c,' '));
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
* Return				: _thread_add((void *)poll_callback,this,(char *)"lm",10);
*******************************************************************************/
int		_LM::DecodeMinus(char *c) {
			switch(*c) {
				case 'P':
					_thread_remove((void *)_LM::Print,this);
					break;
				case 'D':
					for(c=strchr(c,' '); c && *c;)
						_CLEAR_BIT(debug,strtoul(++c,&c,10));
					break;
				case 'E':
					for(c=strchr(c,' '); c && *c;)
						_CLEAR_BIT(error_mask,strtoul(++c,&c,10));
					break;
				case 'f':
					pyro.initFilter();
					break;
				case 'c':
					return ws.ColorOff(strchr(c,' '));
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
* Return				: _thread_add((void *)poll_callback,this,(char *)"lm",10);
*******************************************************************************/
int		_LM::DecodeWhat(char *c) {
			switch(*c) {
				case 'v':
					printf("\r\nV5=%4.1f,V12=%4.1f,V24=%4.1f",_16XtoV5(_ADC::adf.V5),_16XtoV12(_ADC::adf.V12),_16XtoV24(_ADC::adf.V24));			
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
				case 'D':
					printf(" %0*X ",2*sizeof(debug)/sizeof(char),debug);
					break;
				case 'E':
					for(int n=0,e=error; e; e >>= 1, ++n)
						if(_BIT(e, 0))
							printf("\r\nerror %03d: %s",n, ErrMsg[n].c_str());	
						printf("\r\nerror mask=%08X\r\n:",error_mask);	
					break;
				case 'f':
					pyro.printFilter();
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
* Return				: _thread_add((void *)poll_callback,this,(char *)"lm",10);
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
* Return				: _thread_add((void *)poll_callback,this,(char *)"lm",10);
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
				case 'e':
					char s[128];
					if(*++c) {
					int i=strtoul(c,&c,0);
					if(*c) {
						for(int j=0; *c++; s[j]=strtoul(c,&c,0), ++j);
						ee.putPage(i,s);
					} else
						printf("   %s",ee.getPage(i,s));
					} else
						printf("   %s",ee.getSerial(s));
					break;
				case '!': {
					FIL 									f;
					WAVE_FormatTypeDef		w;
					short									nbytes, sample;
					char									flag=0;
					
#ifdef	USE_LCD
int					to=0;
//int					pref=0,
//						peak=0;
#endif
					plot.Clear();
					plot.Add(&plotA,0,5, LCD_COLOR_YELLOW);
					plot.Add(&plotB,0,10, LCD_COLOR_CYAN);
//				plot.Add(&plotC,0,1, LCD_COLOR_YELLOW);

					if(f_open(&f,"0:/3.wav",FA_READ) == FR_OK) {
						if(f_read (&f, &w, sizeof(w), (UINT *)&nbytes)==FR_OK) {
							while(!f_eof(&f)) {
								_wait(3,_thread_loop);
								if(!flag) {
									f_read (&f, &sample, sizeof(sample),(UINT *)&nbytes);
									plotA=sample-6767;
									plotB=pyro.addSample(plotA);

//									if(peak==0) {															// falling..
//										if(plotB < pref) {
//											if(plotB < pref-50) {
//												peak=pref;
//												plotC=0;
//												printf("%d,%d\r\n",to,peak);
//											}
//										}
//										else
//											pref=plotB;
//									} else {																	// rising...
//										if(plotB > pref) {
//											if(plotB > pref + 50)
//												if(peak > 5) {
//													peak=0;
//													plotC=50;
//												}
//										}
//										else {
//											pref=plotB;
//										}
//									}	

#ifdef	USE_LCD
									to=(f_tell(&f)-sizeof(w))*3/2;
									if(plot.Refresh()) {
char								str[16];
										lcd.Grid();
										sprintf(str,"%d",to/1000);
										LCD_SetFont(&Font8x12);
										sFONT *fnt = LCD_GetFont();
										LCD_SetTextColor(LCD_COLOR_GREY);
										LCD_DisplayStringLine(1, (uint8_t *)str);
									}
#endif
								}
								switch(VT100.Escape()) {
									case EOF:
										break;
									case ' ':
										flag ^= 1;
										break;
									case 'l':
										f_lseek(&f, f_tell(&f) - 320);
										break;
									case 'r':
										f_lseek(&f, f_tell(&f) + 320);
										break;
									case 0x1b:
										f_lseek (&f, f.fsize);
										break;
									case __F2:case __f2:
										Select(PLOT_OFFSET);
										break;
									case __F3:case __f3:
										Select(PLOT_SCALE);
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
								}
							}
						f_close(&f);
						}
					} else
						printf("\r\n file not found...\r\n:");
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
bool	_LM::Parse() {
			_stdio(io);
			return Parse(VT100.Escape());
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
			switch(i) {
				case EOF:
					break;

				case __F1:
				case __f1:
					Select(PYRO);
					break;
				case __F2:
				case __f2:
					Select(PILOT);
					break;
				case __F3:
				case __f3:
					Select(PYROnew);
					break;
				case __F4:
				case __f4:
					if(spray.mode.On)
						spray.mode.On=false;
					else
						spray.mode.On=true;
					break;
				case __F5:
				case __f5:
					Select(PUMP);
					VT100.Repeat(1000);
					break;
				case __F6:
				case __f6:
					Select(FAN);
					VT100.Repeat(1000);
					break;			
				case __F7:
				case __f7:
					Select(SPRAY);
					VT100.Repeat(1000);
					break;
				case __F8:
				case __f8:
				{
					_EC20Status		m;
					Select(EC20);
					m.Send(Sys2Ec);
				}
					break;
				case __F9:
				case __f9:
					Select(EC20bias);
					break;				
				case __F10:
				case __f10:
					break;				
				case __F11:
				case __f11:
					FIL f;
					if(f_open(&f,"0:/lm.ini",FA_WRITE | FA_OPEN_ALWAYS) == FR_OK) {
						pyro.SaveSettings((FILE *)&f);
						pump.SaveSettings((FILE *)&f);
						fan.SaveSettings((FILE *)&f);
						spray.SaveSettings((FILE *)&f);
						ec20.SaveSettings((FILE *)&f);
						pilot.SaveSettings((FILE *)&f);
						ws.SaveSettings((FILE *)&f);
						f_sync(&f);
						f_close(&f);							
						printf("\r\n saved...\r\n:");
					}	else				
						printf("\r\n file error...\r\n:");
					break;	

				case __F12:
				case __f12:
					do
						_thread_loop();
					while(ParseCom(io));
					break;
//					return false;
				
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
					RemoteConsole(Can2ComEc20,__CtrlE);
					break;
				case __CtrlF: 
					RemoteConsole(Can2ComIoc,__CtrlF);
					break;	
				case __CtrlV:
					if(spray.vibrate)
						spray.vibrate=false;
					else
						spray.vibrate=true;
					break;

				case __CtrlI:
					_ADC::offset = _ADC::adf;
					printf("\r\n:offset...  %3d,%3d,%3d,%3d\r\n:",pump.offset.cooler,spray.offset.bottle,spray.offset.compressor,spray.offset.air);
					break;

				case __CtrlQ:
					pump.Test();
					break;

				case __CtrlR:
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
					if(_BIT(_LM::debug, DBG_INFO))
						printf("\r\n:\r\n:footswitch disconnected \r\n:");
					spray.mode.On=false;
					ec20.FootSwEvent(__FOOT_OFF);
					break;
				case __FOOT_IDLE:
					if(_BIT(_LM::debug, DBG_INFO))
						printf("\r\n:\r\n:footswitch idle \r\n:");
					spray.mode.On=false;
					ec20.FootSwEvent(__FOOT_IDLE);
					break;
				case __FOOT_MID:
					if(_BIT(_LM::debug, DBG_INFO))
						printf("\r\n:\r\n:footswitch middle \r\n:");
					spray.mode.On=true;
					ec20.FootSwEvent(__FOOT_MID);
					break;
				case __FOOT_ON:		
					if(_BIT(_LM::debug, DBG_INFO))
						printf("\r\n:\r\n:footswitch on \r\n:");
					spray.mode.On=true;
					ec20.FootSwEvent(__FOOT_ON);
					break;
				case __CtrlY:
					NVIC_SystemReset();
				case __CtrlZ:
					while(1);

				default:
					if(!VT100.Line(i))
						break;
					if(int err=Decode(VT100.Line()))
						printf(" ...wtf(%02X)\r\n:",err);
					else
						printf("\r\n:");
					break;
			}
			return true;
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void	_LM::Display(void *v) {
_LM 	*me = static_cast<_LM *>(v);	
_io*	temp=_stdio(me->io);
			while(_buffer_count(me->pyro.buffer) >= 3*sizeof(short)) {
				short 	ta,tp,t;
//______ buffer pull from ISR __________________________________________________					
				_buffer_pull(me->pyro.buffer,&t,sizeof(short));							
				_buffer_pull(me->pyro.buffer,&ta,sizeof(short));
				_buffer_pull(me->pyro.buffer,&tp,sizeof(short));
//______ filter ________________________________________________________________			
				me->plotA=ta;
				me->plotB=me->pyro.addSample(ta+tp);
//______ print at F1____________________________________________________________							
				if(me->pyro.Enabled && me->item == PYRO) {
//					printf("%4d,%5d,%3.1lf,%hu,%u",ta,(int)tp+0x8000,(double)_ADC::Instance()->Th2o/100,t,me->pyro.sync);
					printf("%4d,%5d,%3.1lf,%hu",ta,(int)tp+0x8000,(double)_ADC::Th2o()/100,t);
					printf("\r\n");
				}
				
				if(me->pyro.Enabled && me->item == PYROnew) {
static int		offs=0,cnt=0,sum=0;	
	
							if(t > 10000) {
								t=10000;
								 me->pyro.sync=__time__ - 10000;
							}

							if(t < 20*me->pyro.Period) {
								if((cnt && t <= me->pyro.Period)) {
									printf(":%d\r\n",sum);
									cnt=sum=0;
								} else {
									sum += (ta/2+tp-offs);
									++cnt;
								}
							} else {
								if(cnt) {
									printf(":%d\r\n",sum);
									cnt=sum=0;
								} else
										offs=ta/2+tp;
							}								
				}
//______________________________________________________________________________							
#ifdef	USE_LCD
				if(me->plot.Refresh())
					me->lcd.Grid();				
#endif
			}
			_stdio(temp);
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void	_LM::RemoteConsole(int k, int ctrl) {
int		i,j;
char	c[128];
			Select(REMOTE_CONSOLE);
			sprintf(c,"%02X%02X%02X",k,'v','\r');
			can.Send(c);
			do {
				for(i=0; i<8; ++i) {
					j=getchar();
					if(j == EOF || j == ctrl)
						break;
					sprintf(&c[2*i+2],"%02X",j);
				}
				if(i > 0)
					can.Send(c);
				_thread_loop();
			} while (j != ctrl);
			Select(NONE);
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void	_LM::Print(void *v) {
_LM 	*me = static_cast<_LM *>(v);	
_io		*temp=_stdio(me->io);
			printf("%d,%d,%d,%d\r\n",_ADC::adf.cooler,
																_ADC::adf.bottle,
																	_ADC::adf.compressor,
																		_ADC::adf.air);
			_stdio(temp);
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
			do {
				_stdio(NULL);
				_thread_loop();
			} while(lm.Parse()==true);
			return 0;
	}
}
//Q1   +f 0.00229515,0.00459030,0.00229515,1.89738149,-0.90656211
//Q05  +f 0.00219271, 0.00438542, 0.00219271, 1.81269433, -0.82146519

// band +f 0.03207092,0,-0.03207092,1.89866118,-0.93585815
//		  +f 0.16324316,0,-0.16324316,1.64135758,-0.67351367

// high +f 0.98621179,-1.9724235902,0.98621179,1.972233470,-0.97261371

/*
+f 0.15794200,0,-0.15794200,1.65422610,-0.68411599									bp, 10hz

+f 0.02570835,0.05141670,0.02570835,1.35864700,-0.46148042					lp. 10hz
+f 0.00008735,0.00017470,0.00008735,1.96261474,-0.96296415					lp 1Hz
+f 0.00203203,0.00406407,0.00203203,1.81968752,-0.82781567					lp 5 Hz


+f 1.0, 0.00019998012,-0.999800019877, 1.97576083265,  -0.975764811473
+f 1.0,-1.94962512491, 0.949862985499, 1.26448606693,  -0.310384244533
+f 68.4272224754, -112.860738664, 46.3531200753, 0.0,   0.0

+f 1.0, 0.00019998013,-0.999800019866, 1.97163751419,  -0.971642174499
+f 1.0,-1.95436818551, 0.954558994233, 1.26044681223,  -0.309037826299
+f 132.819471678, -224.031891995, 94.1796821905, 0.0,   0.0
*/
