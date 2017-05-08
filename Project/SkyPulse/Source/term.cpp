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
void	_TERM::Repeat(int t) {
			timeout = -(__time__ + t);
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
char	*_TERM::Line(void) {
			return lp;
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
char	*_TERM::Line(char c) {
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
					if(c > 127)
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
int		_TERM::Escape(void) {
int		i=getchar();

			if(i==EOF) {
				if(timeout && (__time__ > abs(timeout))) {
					timeout=0;
					return seq;
					}
			} else if(timeout > 0) {
				seq=(seq<<8) | i;
				if(i=='~' || i=='A' || i=='B' || i=='C' || i=='D') {
					timeout=0;
					return seq;
				}
			} else if(i==__Esc) {
				timeout=__time__+5;
				seq=i;
			} else {
				timeout=0;
				return i;
			}
			return gp.Poll();
}
/**
* @}
*/
