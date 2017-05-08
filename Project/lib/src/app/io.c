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
// struct _io
//
// buffer --  rx					.. input buffer
//						tx					.. output buffer
//						put()				.. output API
//						get()				.. input API
//						gets				.. command line buffer
//						parse				.. command line parser
//______________________________________________________________________________________
//
//	struct _buffer
//																											,------------------<<push *
//					_buf  *				OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO| 	max. len
//							_pull * <<-------------'															  			 
//______________________________________________________________________________________
// buffer initialization
//
_buffer	*_buffer_init(int length) {
				if(length>0) {
					_buffer	*p=calloc(1,sizeof(_buffer));
					if(p) {
						p->_buf=p->_push=p->_pull=calloc(length,sizeof(char));
						p->size=length;
						if(p->_buf)
							return(p);
						free(p);
					}
				}
				return(NULL);
}
// buffer kill
//______________________________________________________________________________________
_buffer	*_buffer_close(_buffer	*p) {
				if(p) {
					free(p->_buf);
					free(p);
				}
				return NULL;
}
//
//			return number of succesfuly pushed items from q[] into buffer
//______________________________________________________________________________________
int			_buffer_push(_buffer *p, void *q, int n) {
char		*r=q, *t=p->_push;
int			i;
				for(i=0; i<n; ++i) {
					if((int)p->_pull - (int)t == 1)
						break;
					if((int)t - (int)p->_pull == p->size-1)
						break;
					*t++ = *r++;
					if(t == &p->_buf[p->size])
						t = p->_buf;
				}
				p->_push=t;
				return(i);
}
//
//			return number of succesfuly pulled items from buffer onto q[]
//______________________________________________________________________________________
int			_buffer_pull(_buffer *p, void *q, int n) {
int			i=0;
char		*t=p->_pull,
				*r=(char *)q;
				while(n-- && t != p->_push) {
					r[i++] = *t++;
					if(t == &p->_buf[p->size])
						t=p->_buf;
				}
				p->_pull=t;
				return(i);				
}
//______________________________________________________________________________________
int			_buffer_count	(_buffer *p) {
				if(p) {
					if(p->_pull <= p->_push)
						return((int)p->_push - (int)p->_pull);
					else
						return(p->size - (int)p->_pull + (int)p->_push);
				}
				return(0);
}
//______________________________________________________________________________________
//
//	stdin prototype
//
int			__get (_buffer *p) {
int			i=0;
				if(_buffer_pull(p,&i,1))
					return i;
				else
					return EOF;
}
//______________________________________________________________________________________
//
//	stdout prototype
//
int			__put (_buffer *p, int c) {
				if(_buffer_push(p,&c,1) == 1)
					return c;
				else
					return EOF;
}
//______________________________________________________________________________________
//
//	io port init instance
//
_io			*_io_init(int rxl, int txl) {
_io			*p=calloc(1,sizeof(_io));
				if(p) {
					p->rx=_buffer_init(rxl);
					p->tx=_buffer_init(txl);
					p->get=__get;
					p->put=__put;
					p->parse=NULL;
					if(p->rx && p->tx)
						return(p);
					if(p->rx)
						free(p->rx);
					if(p->tx)
						free(p->tx);
				}
				return(NULL);
}
//______________________________________________________________________________________
//
//	io kill
//
_io			*_io_close(_io *io) {
				if(io) {
					_buffer_close(io->rx);
					_buffer_close(io->tx);
					if(io->cmdline)
						free(io->cmdline);
					free(io);
				}
				return NULL;
}

//______________________________________________________________________________________
int			_buffer_put(_buffer *p, void *q, int n) {
char		*t=p->_pull;
				while(n--) {
					if(t == p->_buf)
						t = &p->_buf[p->size];
					if(--t == p->_push)
						return EOF;
					*t = ((char *)q)[n];
				}
				p->_pull=t;
				return *(char *)q;
}
//______________________________________________________________________________________
int 		ungets(char *c) {
				if(__STDIN)
					return _buffer_put(__STDIN->rx,c,strlen(c));
				else
					return EOF;
}
//______________________________________________________________________________________
int 		ungetch(int c) {
				if(__STDIN)
					return _buffer_put(__STDIN->rx,&c,1);
				else
					return EOF;
}
/*-----------------------------------------------------------------------*/
/* Get a character from the file                                          */
/* add. to FatFs																												 */
/*-----------------------------------------------------------------------*/
int			f_getc (FIL* fil) {						/* Pointer to the file object */
				UINT br;
				BYTE s[4];
				f_read(fil, s, 1, &br);				/* Write the char to the file */
				return (br == 1) ? s[0] : EOF;/* Return the result */
}
/**
* @}
*/ 
