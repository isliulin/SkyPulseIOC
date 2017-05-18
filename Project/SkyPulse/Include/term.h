#ifndef					TERM_H
#define					TERM_H
#include				"stm32f2xx.h"
#include				"gpio.h"
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
class _TERM {
	private:
		int			seq, timeout;
		char 		lbuf[128],*lp;
		_GPIO		gp;
	public:
		_TERM() {
			seq=timeout=0;
			lp=lbuf;
		};
		char *Cmd(char c);
		char *Cmd(void);
		void Refresh(int);
		int	 Esc(int);
};

#endif
