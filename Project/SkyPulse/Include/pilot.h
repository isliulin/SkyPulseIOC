#ifndef					PILOT_H
#define					PILOT_H
#include				"stm32f2xx.h"
#include				"stdio.h"

class	_PILOT {
	private:
		int		count;
		bool	enabled;

	public:
		_PILOT();
		~_PILOT();
void	Poll(void);
void	On(void)									{enabled=true;	}
void	Off(void)									{enabled=false;	}
int		Value;
int		Increment(int, int);
void	LoadSettings(FILE *);
void	SaveSettings(FILE *);
};

#endif
