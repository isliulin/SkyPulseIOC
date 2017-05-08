#ifndef					MP3_H
#define					MP3_H
#include				"stm32f2xx.h"
#include				<stdlib.h>
#include				<stdio.h>
#include 				"mp3dec.h"
#include 				"ff.h"

#define READBUF_SIZE 4000

class	_MP3 {
		private:
			FIL *file;	
			HMP3Decoder		mp3;
			MP3FrameInfo	mp3inf;
			int 					bytesLeft, bytesRead;
			unsigned char *readPtr,*readBuf;		
			short					*outPtr;
		public:
		_MP3();
		~_MP3();
		
int		Open(char *);
FIL		*Decode(short *);
static
		void	Play(_MP3 *);

};

#endif
