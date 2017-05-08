#include	"app.h"
#include	"stm32f4x7_eth.h"
#include	"netconf.h"
#include	"stm32f4x7_eth_bsp.h"
#include	<stdio.h>
#include	<stdarg.h>

void			*TCP_init(void *, void *, int);
void			ftpd_init(void);
void 			LCD_LED_Init(void);
//______________________________________________________________________________
void			LwipPoll(void) {
					if (ETH_CheckFrameReceived())
						LwIP_Pkt_Handle();
					LwIP_Periodic_Handle(__time__);
}
//______________________________________________________________________________
void			*test(void *);
void			test1(void *);
void			UDP_init(int);
//______________________________________________________________________________
void			TcpServerInit(void) {
	
					ETH_BSP_Config();
					LwIP_Init();
					_thread_add(LwipPoll,NULL,"LwipPoll",0);

					TCP_init(test,NULL,23);	
					ftpd_init();
//				TCP_init(test1,NULL,6756);	
//				UDP_init(54321);
}
//______________________________________________________________________________
int 			dbg_lwip(const char *fmt, ...) {
					if(__dbug) {
_io*				io=_stdio(__dbug);
						__va_list args;   
						va_start(args, fmt	);
						vprintf(fmt, args);
						printf("\r\n");
						va_end(args);
						_stdio(io);
					}
					return 0;
}
//______________________________________________________________________________
void 			*test(void *v) {
					if(v) {
						ParseCom(v);
					} else {
						v=_io_init(256,256);
					}
					return v;
}


























