#ifndef					LCD_H
#define					LCD_H
#include				"stm32f2xx.h"
#include				"stdio.h"
#include				"stdlib.h"
#include				"stmpe811qtr.h"
#include				"stm32f4_discovery_lcd.h"
#include				<vector>
#include				"gpio.h"

#ifndef	__max				
#define __max(a,b)  (((a) > (b)) ? (a) : (b))	
#endif
#ifndef	__min				
#define __min(a,b)  (((a) < (b)) ? (a) : (b))	
#endif

typedef struct
{
  uint32_t   ChunkID;       /* 0 */ 
  uint32_t   FileSize;      /* 4 */
  uint32_t   FileFormat;    /* 8 */
  uint32_t   SubChunk1ID;   /* 12 */
  uint32_t   SubChunk1Size; /* 16*/  
  uint16_t   AudioFormat;   /* 20 */ 
  uint16_t   NbrChannels;   /* 22 */   
  uint32_t   SampleRate;    /* 24 */
  
  uint32_t   ByteRate;      /* 28 */
  uint16_t   BlockAlign;    /* 32 */  
  uint16_t   BitPerSample;  /* 34 */  
  uint32_t   SubChunk2ID;   /* 36 */   
  uint32_t   SubChunk2Size; /* 40 */    

} WAVE_FormatTypeDef;

using						namespace std;
template				<typename Type>	

class	_PLOT {
	class _POINT {
		public:
			Type 		*Plot;
			short		Colour,Offset,Scale;
			void Draw(int x) {
				LCD_SetTextColor(Colour);
				LCD_DrawCircle(x,LCD_PIXEL_HEIGHT/2+100-((*Plot-Offset)/Scale),1);
			}
		};

	private:
		int x,				// current x-axis position
				idx;			// index of plot selected
		vector<_POINT> points;
	
	public:
		_PLOT() {
			x=idx=0;
		};

		~_PLOT() {
		};

		void	Add(Type *type, Type offset, Type scale, short colour) {
			_POINT p;
			p.Plot=type;
			p.Offset=offset;
			p.Scale=scale;
			p.Colour=colour;
			points.push_back(p);
		};
		
		void	Offset(int a, int b) {
			idx= __min(__max(idx+b,0),points.size()-1);
			points[idx].Offset -= a*points[idx].Scale;
			printf("\r:plot offset ");
			for(int i=0; i<points.size(); ++i)
				printf("%6d", points[i].Offset);
			
			printf("\b");
			for(int i=0; i<points.size()-idx-1; ++i)
				printf("\b\b\b\b\b\b");
			};			

		void	Scale(int a, int b) {
			idx= __min(__max(idx+b,0),points.size()-1);
			points[idx].Scale = __max(points[idx].Scale-a,1);
			printf("\r:plot scale ");
			for(int i=0; i<points.size(); ++i)
				printf("%6d", points[i].Scale);
			
			printf("\b");
			for(int i=0; i<points.size()-idx-1; ++i)
				printf("\b\b\b\b\b\b");
			};			
			
		void	Clear(void) {
			points.clear();
		}

		bool	Refresh(void) {
			for(int i = 0; i != points.size(); ++i)
				points[i].Draw(x);
			x = ++x % LCD_PIXEL_WIDTH;
			
			if(x) 
				return false;
			else
				return true;
		}
};



class	_LCD {
		private:
			short x,y;
		public:
			_LCD();
		
void	Home(void);
void	Grid(void);

};

#endif
