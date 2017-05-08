/**
  ******************************************************************************
  * @file    com.c
  * @author  Fotona d.d.
  * @version V1
  * @date    30-Sept-2013
  * @brief	 COM port parsing related functionality
  *
  */

/** @addtogroup PFM6_Application
* @{
*/

#include	"app.h"
#include	"diskio.h"

#include	<ctype.h>
#include	<math.h>
#include	<string.h>
#include	<ff.h>
#include	"limits.h"
int				lpc_parse(char *);
//___________________________________________________________________________
int				DecodeMinus(char *c) {
					char		*cc[8];
					int			m,n,*p;
					switch(*c) {
//__________________________________________________ debug setup _____________
					case 't':
#if	defined(__TCP__)
					TcpServerInit();
#endif
					break;
//__________________________________________________usb host/file/serial_____
					case 'u':
					if(strscan(c,cc,' ')==2) {
						Initialize_host_msc();
						_wait(200,_thread_loop);
						switch(*cc[1]) {
						case 'h':
							USBH_PowerOn();
							USBH_App=USBH_Iap;
							Initialize_host_msc();
							break;
						case 'f':
							USBH_PowerOff();
							Initialize_device_msc();
							break;
						case 's':
							USBH_PowerOff();
							Initialize_device_vcp();
							break;
						}
						break;
					}
					return _PARSE_ERR_SYNTAX;
//__________________________________________________defragment ______________
					case 'P':
					{
						int i;
						Watchdog_init(__IWDGLOW__);
						i=Defragment(0);
						printf("\r\n>Packing sectors... %d%c",i,'%');
						if(i>10)
							Defragment(-1);
						printf(" Done");
						Watchdog_init(__IWDGHIGH__);
					}
					break;
//__________________________________________________defragment ______________
					case 'A':
						_thread_list();
					break;
//__________________________________________________kill lcd output _________
					case 'L':
						putLCD(NULL,EOF);
					break;
//__________________________________________________formatting flash drive___
					case 'F':
						if(strscan(c,cc,' ')<2)
							return _PARSE_ERR_MISSING;
						switch(*cc[1]) {
							case 'u':
								printf("\rFormat usb ...[y/n]");
								do {
									_wait(5,_thread_loop);
									m=fgetc(&__stdin);
								} while(m==-1);
								if(m == 'y')
									f_mkfs(FSDRIVE_USB,1,4096);
								break;

							case 'f':
								printf("\rFormat flash ...[y/n]");
								Watchdog_init(__IWDGLOW__);
							do {
									_wait(5,_thread_loop);
									m=fgetc(&__stdin);
								} while(m==-1);
								if(m == 'y') {			
									printf(" erasing ");
									for(n=0; n<PAGE_COUNT; ++n)
										if(FLASH_Erase(FATFS_START+n*FLASH_Sector_1)==FLASH_COMPLETE)
											printf(".");
										else
											printf("?");
										f_mkfs(FSDRIVE_CPU,1,4096);
								}
								break;
								
							default:
								return _PARSE_ERR_ILLEGAL;
							}
							break;
					case 'l':
							Initialize_LED(cc,strscan(++c,cc,','));
							break;
//__________________________________________________querying flash sector_______________
					case 'q':
						n=strscan(++c,cc,',');
						if(n) {
							for(p=(int *)FATFS_ADDRESS; p[512/4] != -1; p=&p[512/4+1]) {
								if(p[512/4] == getHEX(cc[0],EOF)) {
									for(m=0; m<512; m+=16) {
										printf("\r\n");
										for(n=0,c=(char *)p; n<16; ++n)
											printf("%02X ",(~(c[m+n]))&0xff);
										for(n=0; n<16; ++n) {
											if((((~(c[m+n]))&0xff)>0x20) && (((~(c[m+n]))&0xff)<0x7f))
												printf("%c",(~(c[m+n]))&0xff);
											else
												printf(".");
										}
									}
									printf("\r\n----------------------------------------------------------------");
									for(m=-1;m==-1;m=fgetc(&__stdin))
										_thread_loop();
									if(m==0x1b)
										break;
								}
							}
							break;
						} else
						SectorQuery();
					break;
//__________________________________________________ mode setup _____________
					case 'm':
						n=strscan(++c,cc,',');
						while(n--)
							_CLEAR_MODE(atoi(cc[n]));
					break;
//__________________________________________________ debug setup _____________
					case 'D':
						__dbug=__STDIN;
						n=strscan(++c,cc,',');
						while(n--)
							_CLEAR_DBG(strtol(cc[n],NULL,0));
					break;
//__________________________________________________ delay execution ________
					case 'd':
						if(strscan(c,cc,' ')==2) {
							_wait(atoi(cc[1]),_thread_loop);
						}
					break;
//______________________________________________________________________________________
					default:
							return _PARSE_ERR_SYNTAX;
					}
					return _PARSE_OK;
}
//______________________________________________________________________________________
int				DecodePlus(char *c) {

					char		*cc[8];
					int			n;
					switch(*c) {
//__________________________________________________ mode setup _____________
						case 'm':
							n=strscan(++c,cc,',');
							while(n--)
								_SET_MODE(atoi(cc[n]));
						break;
//__________________________________________________ watchdog setup _____________
						case 'w':
							if(strscan(++c,cc,','))
								Watchdog_init(atoi(cc[0]));
						break;
//__________________________________________________ add lcd output _____________
						case 'L':
							putLCD(NULL,0);
						break;
//______________________________________________________________________________________
						default:
							return _PARSE_ERR_SYNTAX;
					}
					return _PARSE_OK;
}
//______________________________________________________________________________________
int				DecodeEsc(char *c) {
					int cod=0;
					while(*c) cod=(cod<<8)+*c++;
					switch(cod) {
						case 0x31317E:									//F1
						case 0x31327E:									//F2
						case 0x31337E:									//F3
						case 0x31347E:									//F4
						case 0x31357E:									//F5
						case 0x31377E:									//F6
						case 0x31387E:									//F7
						case 0x31397E:									//F8
						case 0x32307E:									//F9
						case 0x32317E:									//F10
						case 0x32337E:									//F11
						case 0x32347E:									//F12
						case 0x00317E:									//Home
						case 0x00347E:									//End
						case 0x00327E:									//Insert
						case 0x00357E:									//PageUp
						case 0x00337E:									//Delete
						case 0x00367E:									//PageDown
						case 0x000041:									//Up
						case 0x000044:									//Left
						case 0x000042:									//Down
						case 0x000043:									//Right
							printf("%08X",cod);
						return _PARSE_OK;
					}
					return _PARSE_ERR_SYNTAX;
}
//______________________________________________________________________________________
int				EnterFile(char *c) {
static
FIL 			*f=NULL;																	// file object pointer												
					if(!f) {																	// first alloc
						f=calloc(1,sizeof(FIL));	
						if(!f)																	// mem error, exit
							return _PARSE_ERR_MEM;
						if(f_open(f,c,FA_READ|FA_WRITE|FA_OPEN_ALWAYS)==FR_OK &&
							f_lseek(f,f_size(f))==FR_OK) {				// open & pointer to eof
								fprintf((FILE *)f,"\r");						// init. needed kwdf???
								__STDIN->parse=EnterFile;				// parser redirect
								return _PARSE_OK;
							} else {
								free(f);														// free & error exit othw
								return _PARSE_ERR_OPENFILE;
							}	
					}
					if(!c)																		// eol parser entry...
						printf("\r\n");	
					else
						switch(*c) {
							case __CtrlD:
								f_sync(f);														// buffer flush, close file and memfree ...
								f_close(f);
								free(f);
								f=NULL;																// null pointer
								__STDIN->parse=DecodeFs;						// parser redirect
								break;
//______________________________________________________________________________________
							default:
								fprintf((FILE *)f,"%s\r\n",c);
						}
					return _PARSE_OK;
}
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
typedef enum  { __LIST,__ERASE } __FACT;

int 			find_recurse (char * dir_name, char *w, int fact) {
DIR				dir;
FILINFO		fno;
TCHAR			lfn[_MAX_LFN + 1];
					fno.lfname = lfn;
					fno.lfsize = sizeof lfn;
					if (f_opendir(&dir,dir_name) != FR_OK)
						return (EXIT_FAILURE);
					while (1) {
						f_readdir(&dir,&fno);
						if (!dir.sect)
							break;
						else {
							char *p;
							if(dir.lfn_idx != (WORD)-1)
								p=fno.lfname;
							else 
								p=fno.fname;
							if (!strcmp (p, "..") || !strcmp (p, "."))
								continue;
							if (snprintf (lfn, sizeof(lfn), "%s/%s", dir_name, p) >= sizeof(lfn))
								return  (EXIT_FAILURE);	
							if (fno.fattrib & AM_DIR)
									find_recurse (lfn,w,fact);
							switch(fact) {
								case __LIST:
									if(wcard(w,p)) {
										char *q=strchr(dir_name,'/');
										++q;
										printf("\r\n%s",lfn);
										if (fno.fattrib & AM_DIR)
											printf("/");
										else
											printf("%*d",32-strlen(lfn),(int)fno.fsize);
									}
								break;
								case __ERASE:
									if(wcard(w,p))
										f_unlink(p);
								}
						}
					}
					if (f_closedir(&dir) != FR_OK)
						return (EXIT_FAILURE);
					return FR_OK;
}
//______________________________________________________________________________________
int				DecodeFs(char *c) {
					char	*p,*sc[8];

static 		DIR		dir;
	
					TCHAR lfn[_MAX_LFN + 1];									// long filename support
					FILINFO	fno;
			
					fno.lfname = lfn;
					fno.lfsize = sizeof lfn;

					if(!c) {
						printf("\r\n");
						if(f_getcwd(lfn,_MAX_LFN)==FR_OK && f_opendir(&dir,lfn)==FR_OK) {
							if(lfn[strlen(lfn)-1]=='/')
								printf("%s",lfn);
							else
								printf("%s/",lfn);
						} else
							printf("?:/"); 		
					} else if(*c) {
//______________________________________________________________________________________
						int n=strscan(c,sc,' ');
						int len=strlen(sc[0]);
//______________________________________________________________________________________
						if(!(strncmp("0:",sc[0],len) && strncmp("1:",sc[0],len))) {
							if(f_chdrive(c)!=FR_OK ||
								f_getcwd(lfn,_MAX_LFN)!=FR_OK ||
									f_opendir(&dir,lfn)!=FR_OK)
										return _PARSE_ERR_SYNTAX;
						__STDIN->parse=DecodeFs;
						}
//__change directory_____________________________________________________________________
						if(!strncmp("cdir",sc[0],len)) {
							if(n == 2) {
								if(f_chdir(sc[1]) != FR_OK )
									return _PARSE_ERR_OPENFILE;
							} else
								return _PARSE_ERR_MISSING;
						}
//__list directory______________________________________________________________________
						if(!strncmp("directory",sc[0],len)) {
							if(n==1)
								sc[1]="*";
							do {
								if(f_readdir(&dir,&fno) != FR_OK)
									return _PARSE_ERR_OPENFILE;	
								if(dir.sect) {
									char *p;
									if(dir.lfn_idx != (WORD)-1)
										p=fno.lfname;
									else
										p=fno.fname;
									if(wcard(sc[1],p)) {
										printf("\r\n%-16s",p);
										if (fno.fattrib & AM_DIR)
											printf("/");
										else
											printf("%d",(int)fno.fsize);								
									}
								}
							} while(dir.sect);
						}
//__delete file________________________________________________________________________
						if(!strncmp("ls",sc[0],len))
							find_recurse(lfn,sc[1],__LIST);
///__delete file________________________________________________________________________
						if(!strncmp("er",sc[0],len))
							find_recurse(lfn,sc[1],__ERASE);
//__delete file________________________________________________________________________
						if(!strncmp("delete",sc[0],len)) {
							if(n==1)
							sc[1]="*";
							do {
								if(f_readdir(&dir,&fno) != FR_OK)
									return _PARSE_ERR_OPENFILE;	
								if(dir.sect) {
									char *p;
									if(dir.lfn_idx != (WORD)-1)
										p=fno.lfname;
									else
										p=fno.fname;
									if(wcard(sc[1],p)) {
										f_unlink(p);			
									}
								}
							} while(dir.sect);
						}
//__rename file_________________________________________________________________________
						if(!strncmp("rename",sc[0],len)) {
							if(n == 2)
								return _PARSE_ERR_SYNTAX;
							if(f_rename(sc[1],sc[2]) != FR_OK)
							return _PARSE_ERR_SYNTAX;
						}
//__type file_________________________________________________________________________
						if(!strncmp("type",sc[0],len)) {
							if(n == 2) {
								FIL	f;
								if(f_open(&f,sc[1],FA_READ)==FR_OK) {
									printf("\r\n");
									while(!f_eof(&f)) 
										printf("%c",f_getc(&f));
									f_close(&f);
								} else
								return _PARSE_ERR_OPENFILE;
							}
						}
//__make directory_____________________________________________________________________
						if(!strncmp("mkdir",sc[0],len)) {
							if(n == 2) {
								if(f_mkdir(sc[1]) != FR_OK)
									return _PARSE_ERR_OPENFILE;
							} else
								return _PARSE_ERR_MISSING;
						}
//__copy file_________________________________________________________________________
						if(!strncmp("copy",sc[0],len)) {
							char f[256];
							FIL f1,f2;
							if(n == 2) {
								p=strchr(sc[1],':');
								if(p++) {
									if(*p=='/')
										++p;
									strcpy(f,p);
								} else
									strcpy(f,sc[1]);
							}
							else
							if(n == 3) {
								strcpy(f,sc[2]);	
							} else
								return _PARSE_ERR_SYNTAX;
							
							if(!strcmp(sc[1],f))
								strcat(f,"_Copy");

							if(f[strlen(f)-1]==':')
								strcat(f,sc[1]);
							if(f_open(&f1,sc[1],FA_READ)!=FR_OK)
								return _PARSE_ERR_OPENFILE;
							if(f_open(&f2,f,FA_CREATE_ALWAYS | FA_WRITE)!=FR_OK) {
								f_close(&f1);
								return _PARSE_ERR_OPENFILE;
							}
							while(!f_eof(&f1))
								if(fputc(fgetc((FILE *)&f1),(FILE *)&f2)==EOF)
									break;
							if(!f_eof(&f1)) {
								f_close(&f1);
								f_close(&f2);
								return _PARSE_ERR_OPENFILE;
							}
							f_close(&f1);
							f_close(&f2);
						}

//__entering new file__________________________________________________________________
						if(!strncmp("file",sc[0],len)) {
							if(n == 2)
								return(EnterFile(sc[1]));
							else
								return _PARSE_ERR_SYNTAX;
						}

//__entering new file__________________________________________________________________
						if(!strncmp("format",sc[0],len)) {
							char *c,fs[256];
							FIL f1,f2;

							if(n < 3)
								return(_PARSE_ERR_MISSING);
							if(f_open(&f1,sc[1],FA_READ)!=FR_OK)
								return _PARSE_ERR_OPENFILE;
							if(f_open(&f2,sc[2],FA_WRITE | FA_OPEN_ALWAYS)!=FR_OK) {
								f_close(&f1);
								return _PARSE_ERR_OPENFILE;
							};
							
							while(fgets(fs,sizeof(fs),(FILE *)&f1))
								for(c=fs;c < fs + strlen(fs)-2; f_putc(strtol(c,&c,16),&f2));
							
							f_close(&f1);
							f_close(&f2);
							return _PARSE_OK;						
						}
//______________________________________________________________________________________
						if(!strncmp(">",sc[0],len)) {
							__STDIN->parse=DecodeCom;
							return(DecodeCom(NULL));
						}
		}
		return _PARSE_OK;
}
//___________________________________________________________________________
int				DecodeCom(char *c) {
					char 		*cc[8];
					int		 	n;
					if(!c)
						printf("\r\n>");
					else
						switch(*c) {
//__________________________________________________SW version query____________________
					case 'v':
						PrintVersion(SW_version);
					break;
//__________________________________________________single interger read/write__________
					case 'B':
						n=strscan(++c,cc,',');
						if(n) {
							if(n>1)
								*(char *)strtol(cc[0],NULL,16)=(char)strtol(cc[1],NULL,16);
							else
								printf(",%02X",*(unsigned char *)strtol(cc[0],NULL,16));
							break;
						}
					return _PARSE_ERR_SYNTAX;
//__________________________________________________single interger read/write__________
					case 'W':
						n=strscan(++c,cc,',');
						if(n) {
							if(n>1)
								*(short *)strtol(cc[0],NULL,16)=(short)strtol(cc[1],NULL,16);
							else
								printf(",%04X",*(unsigned short *)strtol(cc[0],NULL,16));
							break;
						}
						return _PARSE_ERR_SYNTAX;
//__________________________________________________single interger read/write__________
					case 'L':
					n=strscan(++c,cc,',');
					if(n) {
						if(n>1)
							*(int *)strtol(cc[0],NULL,16)=(int)strtol(cc[1],NULL,16);
						else
							printf(",%08X",*(unsigned int *)strtol(cc[0],NULL,16));
						break;
					}
					return _PARSE_ERR_SYNTAX;					
//__________________________________________________S-records read_____________________
					case 'S':
						return(sLoad(c));
//__________________________________________________S-records/trace dump________________
					case 'D':
						n=strscan(++c,cc,',');
						if(n<2)
							return _PARSE_ERR_SYNTAX;
						if(n==3 && *cc[2]=='i') {
							n = (int)getHEX(cc[1],EOF);
							iDump((int *)getHEX(cc[0],EOF),n);
						}
						else {
							n = (int)getHEX(cc[1],EOF);
							sDump((char *)getHEX(cc[0],EOF),n);
						}
						break;
//______________________________________________________________________________________
					case 'w':
						_wait(strtoul(++c,NULL,0),_thread_loop);
						break;
//______________________________________________________________________________________
					case '#':
						strtok(c," ");
						printf(" %d",wcard(strtok(NULL," "),strtok(NULL," ")));
						break;
//______________________________________________________________________________________
					case '@':
						return batch(++c);
//______________________________________________________________________________________
					case 'x':
					{
					int lm(void);
						_thread_remove(_lightshow,NULL);
					return lm();
					}
//______________________________________________________________________________________
					case '-':
						return DecodeMinus(++c);
//______________________________________________________________________________________
					case '+':
						return DecodePlus(++c);
//______________________________________________________________________________________
					case '0':
					case '1':
						return DecodeFs(c);
//______________________________________________________________________________________
					default:
						return _PARSE_ERR_SYNTAX;
				}
			return _PARSE_OK;
}
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
int					USBH_Iap(int call) {	
FATFS				fs0,fs1;
DIR					dir;
FIL					f0,f1;
BYTE 				buffer[1024];   																									//	file copy buffer */
FRESULT 		fr;          																											//	FatFs function common result code	*/
UINT 				br, bw;         																									//	File read/write count */
FILINFO			fno;
TCHAR				*t;
static
int					state=0;

#if _USE_LFN
char 				lfn[_MAX_LFN + 1];   /* Buffer to store the LFN */
						fno.lfname = lfn;
						fno.lfsize = sizeof lfn;
#endif

						if(state==0 && call==0) {
							++state;
							Watchdog_init(__IWDGHIGH__);
							fno.lfname = lfn;																								//	set long filename buffer 
							fno.lfsize = sizeof lfn;
							_RED2(0);_GREEN2(0);_BLUE2(0);_YELLOW2(0);
							if(f_mount(&fs0,FSDRIVE_USB,1)==FR_OK)													// mount usb 
								if(f_mount(&fs1,FSDRIVE_CPU,1)==FR_OK)												// mount flash
									if(f_chdrive(FSDRIVE_USB)== FR_OK)													// go to usb drive
										if(f_chdir("/sync")==FR_OK)																// goto /sync directory
											if(f_opendir(&dir,lfn)==FR_OK) {												// & open it !
												while(f_readdir(&dir,&fno)==FR_OK && dir.sect) {			// scan the files, if end, exit
													if (fno.fattrib & AM_DIR)														// skip if it is a subdirectory 
														continue;
													t = *fno.lfname ? fno.lfname : fno.fname;						// check for long filenames
													
													if(f_chdrive(FSDRIVE_USB)== FR_OK && f_open(&f0,t,FA_OPEN_EXISTING | FA_READ)!=FR_OK) continue;
													if(f_chdrive(FSDRIVE_CPU)== FR_OK && f_open(&f1,t,FA_CREATE_ALWAYS | FA_WRITE)!=FR_OK) continue;

													for (;;) {
														fr = f_read(&f0, buffer, sizeof buffer, &br);			/* Read a chunk of source file */
														if (fr || br == 0) break; 												/* error or eof */
														fr = f_write(&f1, buffer, br, &bw);								/* Write it to the destination file */
														if (fr || bw < br) break;													/* error or disk full */
														Watchdog();
													}
													++state;
												f_close(&f0);																					// close both files
												f_close(&f1);	
												}
											f_mount(NULL,FSDRIVE_USB,1);														// dismount both drives
											f_mount(NULL,FSDRIVE_CPU,1);
											}
											if(state>1)
												_YELLOW2(1000);
											else
												_RED2(1000);
						}
						
						if(call==EOF) {
							if(state>1)
								WWDG_init();
							state=0;
						}
						return(0);
					}
/**
* @}
*/
