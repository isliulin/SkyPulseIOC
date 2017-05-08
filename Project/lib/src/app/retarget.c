/*----------------------------------------------------------------------------
 * Name:    Retarget.c
 * Purpose: 'Retarget' layer for target-dependent low level functions
 * Note(s):
 *----------------------------------------------------------------------------
 * This file is part of the uVision/ARM development tools.
 * This software may only be used under the terms of a valid, current,
 * end user licence from KEIL for a compatible version of KEIL software
 * development tools. Nothing else gives you the right to use this software.
 *
 * This software is supplied "AS IS" without warranties of any kind.
 *
 * Copyright (c) 2012 Keil - An ARM Company. All rights reserved.
 *----------------------------------------------------------------------------*/
#include <stdio.h>
#include <rt_misc.h>
#include <stdlib.h>
#include <string.h>
#include "stm32f2xx.h"
#include "ff.h"
#include "io.h"
//_________________________________________________________________________________
int 		f_getc (FIL*);
void		_wait(int,void (*)(void)),
				_thread_loop(void);
//_________________________________________________________________________________
FILE 		__stdout;
FILE 		__stdin;
FILE 		__stderr;
//_________________________________________________________________________________
_io			*_stdio(_io	*p) {
_io			*io=__STDIN;
				__STDIN=__STDOUT=p;
				return(io);
}
//__________________________________________________________________________________
int 		fputc(int c, FILE *f) {
				if(f==stdout) {
					if(f->io) {
						while(f->io->put(f->io->tx, c) == EOF)
							_wait(2,_thread_loop);
						if(f->io->file) {
							f_putc(c,f->io->file);
						}
					}
					return c;
				}
				return f_putc(c,(FIL *)f);
}
//__________________________________________________________________________________
int 		fgetc(FILE *f) {
int			c=EOF;
				if(f==stdin) {
					if(f->io) {
						c=f->io->get(f->io->rx);
						if(f->io->file && c==EOF)
							c=f_getc(f->io->file);
					}
					return c;
				}
				return f_getc((FIL *)f);
}
//_________________________________________________________________________________
int 		fclose(FILE* f) 
{
				return((int)f_close(f->io->file));
}
//_________________________________________________________________________________
int 		feof(FILE* f) 
{	
				return((int)f_eof(f->io->file));
}
//_________________________________________________________________________________
FILE 		*fopen(const char *filename, const char *att) {
				int oflag=0;
				int offset=0;
				FIL *f=malloc(sizeof(FIL));
	
				if(!f)
					return(NULL);
				switch (*att) {
					case 'r':
						oflag |= FA_READ;
						break;
					case 'w':
						oflag = FA_WRITE | FA_CREATE_ALWAYS;
						break;
					case 'a':
						oflag = FA_WRITE | FA_CREATE_ALWAYS;
						offset=-1;
						break;
					default:
						return NULL;
				}

				while (*++att) {
					switch (*att) {
						case '+':
							oflag |= FA_READ | FA_WRITE;
							oflag &= ~(FA_CREATE_ALWAYS);
							break;
						case 't':
							break;
						case 'b':
							break;
						case 'c':
						case 'n':
							break;
						case 'S':
							break;
						case 'R':
							break;
						case 'T':
							break;
						case 'D':
							break;
						case ' ':
							break;
						default:
							return NULL;
					}
				}
				if(f_open(f,"filename",oflag) == FR_OK) {
					if(offset<0)
						f_lseek(f,f_size(f));
					else
						f_lseek(f,offset);
					return (FILE *)f;
				} else {
					free(f);
					return NULL;
				}
} 
//_________________________________________________________________________________
FILE 		*freopen(const char *filename, const char *mode, FILE *stream)
{ 
				fclose(stream);
				return fopen(filename,mode);
}
//_________________________________________________________________________________
int 		fseek (FILE *f, long nPos, int nMode)  {
				switch(nMode) {
					case SEEK_SET:
						return(f_lseek(f->io->file,nPos));
					case SEEK_CUR:
						return(f_lseek(f->io->file, f_tell(f->io->file)+nPos));
					case SEEK_END:
						return(f_lseek(f->io->file, f_size(f->io->file)-nPos));
					default:
						return EOF;
				}
}
//_________________________________________________________________________________
int 		fflush (FILE *f)  {
				return	f_sync(f->io->file);
}
//_________________________________________________________________________________
int 		ferror(FILE *f) {
				return	f_error(f->io->file);
}
//_________________________________________________________________________________
void 		_ttywrch(int c) {
				fputc(c,&__stdout);
}
//_________________________________________________________________________________
void		_sys_exit(int return_code) {
label:  goto label;  /* endless loop */
}
//_________________________________________________________________________________
#include        <time.h>
#include        <limits.h>
/*
* loc_time.h - some local definitions
*/
/* $Header: /users/cosc/staff/paul/CVS/minix1.7/src/lib/ansi/loc_time.h,v 1.2 1996/04/10 21:04:30 paul Exp $ */

#define YEAR0           1900                    /* the first year */
#define EPOCH_YR        1970            /* EPOCH = Jan 1 1970 00:00:00 */
#define SECS_DAY        (24L * 60L * 60L)
#define LEAPYEAR(year)  (!((year) % 4) && (((year) % 100) || !((year) % 400)))
#define YEARSIZE(year)  (LEAPYEAR(year) ? 366 : 365)
#define FIRSTSUNDAY(timp)       (((timp)->tm_yday - (timp)->tm_wday + 420) % 7)
#define FIRSTDAYOF(timp)        (((timp)->tm_wday - (timp)->tm_yday + 420) % 7)
#define TIME_MAX        ULONG_MAX
#define ABB_LEN         3

const int _ytab[2][12] = {
                 { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
                 { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
         };
extern const char *_days[];
extern const char *_months[];

void _tzset(void);
unsigned _dstget(struct tm *timep);

extern long _timezone;
extern long _dst_off;
extern int _daylight;
extern char *_tzname[2];
struct tm *
gmtime(register const time_t *timer)
{
static  struct tm *timep = NULL;
        time_t time = *timer;
        unsigned long dayclock, dayno;
        int year = EPOCH_YR;
	
				if(timep==NULL)
					timep=malloc(sizeof(struct tm));

        dayclock = (unsigned long)time % SECS_DAY;
        dayno = (unsigned long)time / SECS_DAY;

        timep->tm_sec = dayclock % 60;
        timep->tm_min = (dayclock % 3600) / 60;
        timep->tm_hour = dayclock / 3600;
        timep->tm_wday = (dayno + 4) % 7;       /* day 0 was a thursday */
        while (dayno >= YEARSIZE(year)) {
                dayno -= YEARSIZE(year);
                year++;
        }
        timep->tm_year = year - YEAR0;
        timep->tm_yday = dayno;
        timep->tm_mon = 0;
        while (dayno >= _ytab[LEAPYEAR(year)][timep->tm_mon]) {
                dayno -= _ytab[LEAPYEAR(year)][timep->tm_mon];
                timep->tm_mon++;
        }
        timep->tm_mday = dayno + 1;
        timep->tm_isdst = 0;

        return timep;
}
