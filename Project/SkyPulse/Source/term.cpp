#include "term.h"
#include "isr.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
/**
******************************************************************************
* @file
* @author  Fotona d.d.
* @version
* @date
* @brief	 
*
*/
/** @addtogroup
* @{
*/
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void	_TERM::Refresh(int t, int ch) {
			refresh.seq = ch;
			refresh.timeout = __time__ + t;
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
char	*_TERM::Cmd(void) {
			return lp;
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
char	*_TERM::Cmd(int c) {
			switch(c) {
				case 0x08:
				case 0x7f:
					if(lp != lbuf) {
						--lp;
					printf("\b \b");
					}
					break;

				case 0x0d:
					*lp=0;
					lp=lbuf;		
					return lp;

				case EOF:
				case 0x0a:
					break;

				default:
					if(c < ' ' || c > 127)
						printf("<%X>",c);
					else {
						printf("%c",c);
						*lp++=c;
					}
				}
			return NULL;
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
int		_TERM::Esc(int i) {
			if(i==EOF) {
				if(esc.timeout && (__time__ > esc.timeout)) {
					esc.timeout=refresh.timeout=0;
					return esc.seq;
					}
				if(refresh.timeout && (__time__ > refresh.timeout)) {
					refresh.timeout=0;
					return refresh.seq;
					}
			} else if(esc.timeout > 0) {
				esc.seq=(esc.seq<<8) | i;
				if(i=='~' || i=='A' || i=='B' || i=='C' || i=='D') {
					esc.timeout=refresh.timeout=0;
					return esc.seq;
				}
			} else if(i==__Esc) {
				esc.timeout=__time__+5;
				refresh.timeout=0;
				esc.seq=i;
			} else {
				esc.timeout=refresh.timeout=0;
				return i;
			}
			return gp.Poll();
}
/**
* @}
*/
