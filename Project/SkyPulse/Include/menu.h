#ifndef					MENU_H
#define					MENU_H
#include				"stm32f2xx.h"
#include				<stdio.h>
#include 				<string.h>

#define		MAX_ITEMS	4

class	_MENU {
	
	private:
	int			item;
	char		*str;
	int			(*f)(void *);
	void		*arg;
	_MENU		*next[MAX_ITEMS];
	
	public:
	_MENU(char *,_MENU *,_MENU *,_MENU *,_MENU *);
	_MENU(int(*)(void *), void *);
	void		Refresh();
	int			Poll(int);
};

#endif
