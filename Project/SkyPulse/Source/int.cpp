/**
	******************************************************************************
	* @file		regs.cpp
	* @author	Fotona d.d.
	* @version
	* @date		
	* @brief	Timers initialization & ISR
	*
	*/	
/** @addtogroup
* @{
*/
#include	"int.h"
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
/*******************************************************************************
* Function Name	:
* Description		: 
* Output				:
* Return				: None
*******************************************************************************/
_INT::_INT() {



}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
int		_INT::Poll(void *v) {
			_INT *self = static_cast<_INT *>(v);
			if(Src)
				Value += (Ref - *Src)/Gain;
			else
				Value += Ref/Gain;
			if(Dst)
				*Dst=Value;
			return Value;
}
/*


*/
