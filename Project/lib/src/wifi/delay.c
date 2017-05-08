#include	<string.h>
#include	<stdio.h>
#include 	<stdarg.h>
#include 	"io.h"
#include 	"delay.h"
//___________________________________________________________________________________________
extern		uint8_t 	seqNo;
extern		rx_info_t	rx_frame[];
extern 		_buffer	*Dbg;
extern 		_io* __com0;

int				curr_size=-1,curr_sock=-1;
//___________________________________________________________________________________________
int				__sendFromSock() {
uint8_t 	buf[128];
static		int	n=0;
					if(n)
						return 0;	
					else
						n=_buffer_pull(__com0->tx,buf,128);
					if(n && curr_sock != -1)
						sendFromSock(curr_sock, buf, n, 2, seqNo++);						
					n=0;
					return 0;
}
//___________________________________________________________________________________________
void			mdelay(u32 ms)
{
					Wait(ms,App_Loop);
}
//___________________________________________________________________________________________
int 			DbgScan(const char *f, ...) {
	
					char p[128];
					int	i;
					va_list argptr;

					for(i=0; i<128; ++i) {
						if(_buffer_empty(Dbg))
							return 0;
						_buffer_pull(Dbg,&p[i],1);
						if(!p[i] || p[i]=='\r' || p[i]=='\n')
							break;
					}
					p[i]='\0';										// yay, just in case....
					va_start(argptr, f);
					i=vsscanf(p,f, argptr);
					va_end(argptr);
					return i;
}
//___________________________________________________________________________________________
int 			DbgPrint(const char *f, ...) {
int 			i,j;
static
_io				*io=NULL;
					if(!f) {
						io=__stdin.handle.io;
						return 0;
					}
					
					if(io) {
						va_list 	p;
						va_start(p, f);
						io=_stdio(io);
						vprintf(f, p);
						io=_stdio(io);
						va_end(p);
					}
									
					if(!strncmp("%d bytes received",f,17)) {
						va_list 	p;
						va_start(p, f);
						curr_size=va_arg(p,int);
						curr_sock=va_arg(p,int);
					}					
					if(!strcmp(f,".\r\n") && curr_size) {
						for(j=0; j<NUM_RX_BUF; ++j)
							if(rx_frame[j].available==true && rx_frame[j].rx_payload[2]==curr_sock)
								break;
						for (i=0; i<curr_size; ++i) {
							while(_buffer_push(__com0->rx,&rx_frame[j].rx_payload[5+i],1)==EOF)
								mdelay(1);
						}
						curr_size=0;
					}
					return 0;
}
