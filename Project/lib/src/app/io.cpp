/**
  ******************************************************************************
  * @file    io.cpp
  * @author  Fotona d.d.
  * @version V1
  * @date    30-Sept-2013
  * @brief	 System I/O
  *
  */ 
	
/** @addtogroup PFM6_Misc
* @brief PFM6 miscellaneous
* @{
*/
#include 	<stdio.h>
//______________________________________________________________________________
class cBuffer
{
	private:
			char	*_buffer,*_push,*_pull;
			int		_length;	
	public:
		cBuffer(int length=128) 
		{
			_push=_pull=_buffer=new char[length];
		}
		
		int	push(int c)	 
		{
			char	*p=this->_push;
			if(++p == &this->_buffer[this->_length])
				p = this->_buffer;
			if(p == this->_pull)
				c=EOF;
			else {
				*this->_push = (char)c;
				this->_push=p;
			}
			return(c);
		}

		int	push(char *c)	 
		{
			while(*c)
				if(push(*c++)==EOF)
					return(EOF);
			return(*c);
		}
		
		int	push(char *c, int n)	 
		{
			while(n--)
				if(push(*c++)==EOF)
					return(EOF);
			return(*c);
		}
		
		int	pull()
		{
			int		i;
			char	*p=this->_pull;
			if(p != this->_push) {
				i = *p;
				if(++p == &this->_buffer[this->_length])
					p = this->_buffer;
				this->_pull=p;
				return(i);
			}
			else
				return(EOF);
		}
		
		bool empty()
		{
			return (this->_push == this->_pull);
		}
};
//______________________________________________________________________________
class cIO
{
	private:
		cBuffer	*rx,*tx;

	public:
		cIO(int rxLen, int txLen) 
		{
			rx= new cBuffer(rxLen);
			tx= new cBuffer(txLen);
		}
};
//______________________________________________________________________________
extern "C" 
{
cBuffer* 	buffer(void* p) { 
					return static_cast<cBuffer*>(p); 
}

void			*_buffer_init(int length) {
						return(new cBuffer(length));
}
//______________________________________________________________________________
int				_buffer_empty(void *p) {
						return buffer(p)->empty();
}
//______________________________________________________________________________
int				_buffer_push(void *p,char *c,int i) {
					return buffer(p)->push(c,i);
}


}



	
/**
* @}
*/ 

