#ifndef LPC_H
#define LPC_H

#define MAX_LPC_ORDER 32

#define ORDER_METHOD_MAX    0
#define ORDER_METHOD_EST    1
#define ORDER_METHOD_2LEVEL 2
#define ORDER_METHOD_4LEVEL 3
#define ORDER_METHOD_8LEVEL 4
#define ORDER_METHOD_SEARCH 5
#include	"stm32f2xx.h"
extern		"C"  {	
int lpc_calc_coefs(int32_t *samples, int blocksize, int max_order,
               int precision, int omethod, int32_t coefs[][MAX_LPC_ORDER],
               int *shift);
}
#endif /* LPC_H */

