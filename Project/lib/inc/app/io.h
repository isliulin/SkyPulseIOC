#ifndef _IO_H
#define _IO_H

#include		<stdio.h>
#include		<stdlib.h>
#include		"ffconf.h"
#include		"ff.h"
//______________________________________________________________________________________
typedef struct _buffer
{
	char	*_buf, *_push, *_pull;
	int		(* push)(struct _buffer *, char *);
	int		(* pull)(struct _buffer *);
	int		size;
} _buffer;	
//______________________________________________________________________________________
typedef struct _io
{
_buffer	*rx,
				*tx,
				*cmdline;
int			(*get)(_buffer *),
				(*put)(_buffer *, int);
int			(*parse)(char *);
				FIL 		*file;
} _io;
//______________________________________________________________________________________
_buffer	*_buffer_init(int),
				*_buffer_close(_buffer *);
	
_io			*_io_init(int, int),
				*_io_close(_io *),
				*_stdio(_io	*);

int			_buffer_push(_buffer *, void *,int),
				_buffer_put(_buffer *, void *,int),
				_buffer_pull(_buffer *, void *,int),
				_buffer_count(_buffer *);
				
int			ungets(char *);
int 		ungetch(int);
	
int 		f_getc (FIL*);		
void		Watchdog(void);
				
#define __STDIN				stdin->io
#define __STDOUT			stdout->io
struct	__FILE
{
	_io		*io;
};
#endif
