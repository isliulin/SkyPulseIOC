#include		<stdio.h>
#include		<stdlib.h>
#include		"ffconf.h"
#include		"ff.h"

#ifndef _IO_H
#define _IO_H
//______________________________________________________________________________________
typedef struct _buffer
{
	char	*_buf,
				*_push,
				*_pull;
	int		(* push)(struct _buffer *, char *);
	int		(* pull)(struct _buffer *);
	int		len;
} _buffer;	
//______________________________________________________________________________________
typedef struct _io
{
	_buffer	*rx,*tx,*gets;
	int		(*get)(struct _buffer *),
				(*put)(struct _buffer *, int);
	int			(*parse)(char *),
				flags;
} _io;
//______________________________________________________________________________________
_buffer	*_buffer_init(int);
_io			*_io_init(int, int);
_io			*_stdio(_io	*);

int			_buffer_push(_buffer *, void *,int),
				_buffer_pull(_buffer *, void *,int),
				_buffer_empty(_buffer *),
				_buffer_LIFO(_buffer *, void *, int);
				
int			putch(int),
				getch(void),
				ungetch(int),
				ungets(char *);
				
void		Watchdog(void);

struct	__FILE 
{ 
				union {
					_io		*io;
					FIL 	*fil; 
					FILE	*file; 
				} handle;
				int err;
};
#endif
