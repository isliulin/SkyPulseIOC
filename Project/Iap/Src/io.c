/**
  ******************************************************************************
  * @file    io.c
  * @author  Fotona d.d.
  * @version V1
  * @date    30-Sept-2013
  * @brief	 System I/O
  *
  */ 
/** @addtogroup PFM6_Misc
* @brief PFM6 miscellaneous
* @{
*/
#include 	<stdio.h>
#include 	<stdlib.h>
#include 	<string.h>
#include 	"io.h"
//______________________________________________________________________________________
_buffer	*_buffer_init(int length) {
_buffer	*p=calloc(1,sizeof(_buffer));

				if(p) {
					p->_buf=p->_push=p->_pull=calloc(length,sizeof(char));
					p->len=length;
					if(p->_buf)
						return(p);					
				}
				return(NULL);
}
//___push_______________________________________________________________________________
int			_buffer_push(_buffer *p, void *q, int n) {
char		*r=q,
				*t=p->_push;
				while(n--) {
					*t++ = *r++;
					if(t == &p->_buf[p->len])
						t = p->_buf;
					if(t == p->_pull)
						return EOF;
				}
				p->_push=t;
				return(r[-1]);
}
//___pull_______________________________________________________________________________
int			_buffer_pull(_buffer *p, void *q, int n) {
int			i=0;
char		*t=p->_pull,
				*r=(char *)q;
				while(n-- && t != p->_push) {
					r[i++] = *t++;
					if(t == &p->_buf[p->len])
						t=p->_buf;
				}
				p->_pull=t;
				return(i);				
}
//___test empty_________________________________________________________________________
int			_buffer_empty	(_buffer *p) {
				if(p->_pull != p->_push)
					return(0);
				else
					return(EOF);
}
//______________________________________________________________________________________
_io			*_io_init(int rxl, int txl) {
_io			*p=calloc(1,sizeof(_io));
				if(p) {
					p->flags=0;
					p->rx=_buffer_init(rxl);
					p->tx=_buffer_init(txl);
					if(p->rx && p->tx)
						return(p);				
				}
				return(NULL);
}
//______________________________________________________________________________________
int			_buffer_LIFO(_buffer *p, void *q, int n) {
char		*t=p->_pull;
				while(n--) {
					if(t == p->_buf)
						t = &p->_buf[p->len];
					if(--t == p->_push)
						return EOF;
					*t = ((char *)q)[n];
				}
				p->_pull=t;
				return *(char *)q;
}
//______________________________________________________________________________________
int 		ungetch(int c) {
				if(__stdin.handle.io)
					return _buffer_LIFO(__stdin.handle.io->rx,&c,1);
				else
					return EOF;
}
//______________________________________________________________________________________
int 		ungets(char *c) {
				if(__stdin.handle.io)
					return _buffer_LIFO(__stdin.handle.io->rx,c,strlen(c));
				else
					return EOF;
}
/**
* @}
*/ 
