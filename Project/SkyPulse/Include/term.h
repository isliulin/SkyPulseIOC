#ifndef					TERM_H
#define					TERM_H
#include				"stm32f4xx.h"
#include				"gpio.h"
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
class _TERM {
	private:
		struct {	
			uint32_t	seq;
			uint32_t	timeout;
		} esc, refresh;
		
		char 		lbuf[128],*lp;
		_GPIO		gp;
	public:
		_TERM() {
			esc.seq=esc.timeout=0;
			refresh.seq=refresh.timeout=0;
			lp=lbuf;
		};
		char *Cmd(int c);
		char *Cmd(void);
		void Refresh(int, int);
		int	 Esc(int);
};

#endif
