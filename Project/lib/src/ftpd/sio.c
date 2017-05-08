#include	<stm32f2xx.h>
#include "lwip/sio.h"
#include <stdio.h>
#if LWIP_HAVE_SLIPIF
int				putVCP (int	);
int				getVCP (void);
//_____________________________________________________________________________________
sio_fd_t sio_open(u8_t devnum) {
	
	return(sio_fd_t) devnum;
}
//_____________________________________________________________________________________
u32_t sio_read(sio_fd_t fd, u8_t *data, u32_t len) {
int n=0;
		do
			n+=sio_tryread(fd,data,len-n);
		while(n<len);
		return(len);
}
//_____________________________________________________________________________________
u32_t sio_tryread(sio_fd_t fd, u8_t *data, u32_t len) {
	int	n,c;
	do {
		n=0;
		c=getVCP();
		if(c==-1)
			break;
		data[n++]=c & 0xff;
	} while(n < len);
	return(n);
}
//_____________________________________________________________________________________
void sio_send(u8_t c, sio_fd_t fd) {
	putVCP (c);
}
//_____________________________________________________________________________________
u32_t sio_write(sio_fd_t fd, u8_t *data, u32_t len) {
	return len;
}
//_____________________________________________________________________________________
void sio_read_abort(sio_fd_t fd) {
}
#endif
