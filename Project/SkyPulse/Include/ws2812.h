#ifndef					WS2812_H
#define					WS2812_H
#include 				<string>
#include				<stdlib.h>
#include				<stdio.h>

typedef struct	{unsigned char r; unsigned char g; unsigned char b; }	RGB_set;
typedef struct	{
	signed short	h;
	unsigned char s;
	unsigned char v; 
}	HSV_set;

inline bool operator == (HSV_set &a, HSV_set &b) {
    return a.h == b.h && a.s == b.s && a.v == b.v;
}

inline bool operator != (HSV_set &a, HSV_set &b) {
    return !(a==b);
}

#define	_TP		74	//104 // 
#define _TH		53	//75	// 
#define _TL		20	//29	// 

typedef enum		{ noCOMM,
										SWITCH_ON, SWITCH_OFF, 
										FILL_ON, FILL_OFF, 
										FILL_LEFT_ON, FILL_RIGHT_ON, 
										FILL_LEFT_OFF, FILL_RIGHT_OFF,
										RUN_LEFT_ON, RUN_RIGHT_ON,
										RUN_LEFT_OFF, RUN_RIGHT_OFF
								}	wsCmd;

typedef struct	{
#ifdef __IOC_V2__		
									short g[8][2];
									short r[8][2];
									short b[8][2];
#else
									int g[8];
									int r[8];
									int b[8];
#endif
} dma;

typedef	struct	{
									int				size;
									HSV_set		color, *cbuf;
									wsCmd			mode;
									dma 			*lbuf;
								} ws2812;

class	_WS2812 {
	private:
		void 		RGB2HSV( RGB_set, HSV_set *);
		void		HSV2RGB( HSV_set, RGB_set *);
		void		trigger(void);
		dma			*dma_buffer;
		int			dma_size;
		static 	ws2812 	ws[];
//______________________________________________________________________________________
	public:
		_WS2812(void);
		~_WS2812(void);
		int						ColorOn(char *);
		int						ColorOff(char *);
		int						SetColor(char *);
		int						GetColor(int);
		void					Cmd(int,wsCmd);
		void					SaveSettings(FILE *);
		static void		*proc_WS2812(_WS2812 *);
};

#endif
