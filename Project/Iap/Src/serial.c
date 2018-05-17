#include	"iap.h"
//______________________________________________________________________________________
//ava za gets, ne caka, vraca pointer na string brez \r(!!!) ali NULL	
// èe je 
// zamenjmode ECHO (-1) na
//							<cr> izpiše <cr><lf>
//							<backspace> ali <del> izpiše <backspace><space><backspace>	
//
//______________________________________________________________________________________
char	*cgets(int c, int mode)
{
_buffer		*p=__stdin.handle.io->gets;
			
			if(!p)
				p=__stdin.handle.io->gets=_buffer_init(__stdin.handle.io->rx->len);
			switch(c) {
				case EOF:		
					break;
				case '\r':
				case '\n':
					*p->_push = '\0';
					p->_push=p->_pull=p->_buf;
					return(p->_buf);
				case 0x08:
				case 0x7F:
					if(p->_push != p->_pull) {
						--p->_push;
						if(mode)
							printf("\b \b");
					}
					break;
				default:
					if(p->_push != &p->_buf[p->len-1])
						*p->_push++ = c;
					else  {
						*p->_push=c;
						if(mode)
							printf("\b");
					}
					if(mode) {
						if(isprint(c))
							printf("%c",c);
						else
							printf("%c%02X%c",'<',c,'>');
					}
					break;
			}
			return(NULL);
}
//______________________________________________________________________________________
void	ParseCOM(void) {
int		i,j;
char	*p;

			switch(getchar()) {			
				case _CtrlY:
					NVIC_SystemReset();
				case _CtrlZ:
					while(1);
				case '1':
					for(j=0; j<5; ++j) {
					CanHexMessage(_ID_IAP_ERASE,_SIGN_PAGE+j*_PAGE_SIZE);
					for(i=0;i<1000;++i)	
						App_Loop();
					}
					break;
				case '2':
					CanHexMessage(_ID_IAP_SIGN,0);
					break;
				case '9':
					CanHexMessage(_ID_IAP_SIGN,9);
					break;
				case '3':
					CanHexMessage(_ID_IAP_GO,0);
					break;
				case '?':
					RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_CRC, ENABLE);
					CRC_ResetDR();
					printf("\rPFM6 bootloader v%d.%02d %s, <%08X>\r\n",
						SW_version/100,SW_version%100,__DATE__,CRC_CalcBlockCRC(__Vectors, ((_FLASH_TOP-28)-(int)__Vectors)/sizeof(int)));			//crc od vektorjev do zacetka FS
					printf("signature %08X:\r\n",_FLASH_TOP);
					p=(char *)_SIGN_CRC;
					for(i=0;i<8;++i)
						printf("%08X\r\n",*(int *)p++);
					break;
				case 0x1b:
					if(!(__CAN__->BTR & ((CAN_Mode_LoopBack)<<30)))
						Initialize_CAN(1);
					CanHexMessage(_ID_IAP_PING,0);
					break;
				case ':':
					do p=cgets(getchar(),0); while(!p);
					strcpy(_Iap_string,p);
					CanHexProg(NULL);
					break;
				default:
					break;
			}
}
//___________________________________________________________________________
extern	volatile int __time__;
void		Wait(int t,void (*f)(void)) {
int			to=__time__+t;
				while(to > __time__)
					if(f)
						f();
}
