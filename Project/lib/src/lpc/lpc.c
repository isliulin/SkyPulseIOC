
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <math.h>

#include <stdlib.h>
#include <inttypes.h>

void apply_welch_window(const int32_t *data, int len, double *w_data)
{
    int i, n2;
    double w;
    double c;

    n2 = (len >> 1);
    c = 2.0 / (len - 1.0);
       for(i=0; i<n2; i++) {
              w = c - i - 1.0;
        w = 1.0 - (w * w);
        w_data[i] = data[i] * w;
        w_data[len-1-i] = data[len-1-i] * w;
       }
}

void compute_autocorr(const int32_t *data, int len, int lag, double *autoc)
{
    int i;
    double *data1;
    int lag_ptr, ptr;

    data1 = (double *)malloc(len * sizeof(double));
    apply_welch_window(data, len, data1);

    for(i=0; i<lag; i++) autoc[i] = 1.0;

    ptr = 0;
    while(ptr <= lag) {
        lag_ptr = 0;
        while(lag_ptr <= ptr) {
            autoc[ptr-lag_ptr] += data1[ptr] * data1[lag_ptr];
            lag_ptr++;
        }
        ptr++;
    }
    while(ptr < len) {
        lag_ptr = ptr - lag;
        while(lag_ptr <= ptr) {
            autoc[ptr-lag_ptr] += data1[ptr] * data1[lag_ptr];
            lag_ptr++;
        }
        ptr++;
    }

    free(data1);
}


void compute_lpc_coefs(const double *autoc, int max_order,
                  double lpc[][MAX_LPC_ORDER], double *ref)
{
   int i, j, i2;
   double r, err, tmp;
   double lpc_tmp[MAX_LPC_ORDER];

   for(i=0; i<max_order; i++) lpc_tmp[i] = 0;
   err = autoc[0];

   for(i=0; i<max_order; i++) {
      r = -autoc[i+1];
      for(j=0; j<i; j++) {
          r -= lpc_tmp[j] * autoc[i-j];
      }
      r /= err;
      ref[i] = fabs(r);

      err *= 1.0 - (r * r);

      i2 = (i >> 1);
      lpc_tmp[i] = r;
      for(j=0; j<i2; j++) {
         tmp = lpc_tmp[j];
         lpc_tmp[j] += r * lpc_tmp[i-1-j];
         lpc_tmp[i-1-j] += r * tmp;
      }
      if(i % 2) {
          lpc_tmp[j] += lpc_tmp[j] * r;
      }

      for(j=0; j<=i; j++) {
          lpc[i][j] = -lpc_tmp[j];
      }
   }
}

void quantize_lpc_coefs(double *lpc_in, int order, int precision, int32_t *lpc_out,
                   int *shift)
{
       int i;
       double d, cmax;
       int32_t qmax;
       int sh, max_shift;

    /* limit order & precision to FLAC specification */
//    assert(order >= 0 && order <= MAX_LPC_ORDER);
//       assert(precision > 0 && precision < 16);

    /* define maximum levels */
    max_shift = 15;
    qmax = (1 << (precision - 1)) - 1;

    /* find maximum coefficient value */
    cmax = 0.0;
    for(i=0; i<order; i++) {
        d = lpc_in[i];
        if(d < 0) d = -d;
        if(d > cmax)
            cmax = d;
    }

    /* if maximum value quantizes to zero, return all zeros */
    if(cmax * (1 << max_shift) < 1.0) {
        *shift = 0;
        for(i=0; i<order; i++) {
            lpc_out[i] = 0;
        }
        return;
    }

    /* calculate level shift which scales max coeff to available bits */
    sh = max_shift;
    while((cmax * (1 << sh) > qmax) && (sh > 0)) {
        sh--;
    }

    /* since negative shift values are unsupported in decoder, scale down
       coefficients instead */
    if(sh == 0 && cmax > qmax) {
        double scale = ((double)qmax) / cmax;
        for(i=0; i<order; i++) {
            lpc_in[i] *= scale;
        }
    }

    /* output quantized coefficients and level shift */
    for(i=0; i<order; i++) {
        lpc_out[i] = (int32_t)(lpc_in[i] * (1 << sh));
    }
    *shift = sh;
}

int estimate_best_order(double *ref, int max_order)
{
    int i, est;

    est = 1;
    for(i=max_order-1; i>=0; i--) {
        if(ref[i] > 0.10) {
            est = i+1;
            break;
        }
    }
    return est;
}

int lpc_calc_coefs(const int32_t *samples, int blocksize, int max_order,
               int precision, int omethod, int32_t coefs[][MAX_LPC_ORDER],
               int *shift)
{
    double autoc[MAX_LPC_ORDER+1];
    double ref[MAX_LPC_ORDER];
    double lpc[MAX_LPC_ORDER][MAX_LPC_ORDER];
    int i, j;
    int opt_order;

    /* order 0 is not valid in LPC mode */
    if(max_order < 1) return 1;

    compute_autocorr(samples, blocksize, max_order+1, autoc);

    compute_lpc_coefs(autoc, max_order, lpc, ref);

    opt_order = max_order;
    if(omethod == ORDER_METHOD_EST || omethod == ORDER_METHOD_SEARCH) {
        opt_order = estimate_best_order(ref, max_order);
    }
    switch(omethod) {
        case ORDER_METHOD_MAX:
        case ORDER_METHOD_EST:
            i = opt_order-1;
            quantize_lpc_coefs(lpc[i], i+1, precision, coefs[i], &shift[i]);
            break;
        case ORDER_METHOD_2LEVEL:
            i = (max_order/2)-1;
            if(i < 0) i = 0;
            quantize_lpc_coefs(lpc[i], i+1, precision, coefs[i], &shift[i]);
            i = max_order-1;
            quantize_lpc_coefs(lpc[i], i+1, precision, coefs[i], &shift[i]);
            break;
        case ORDER_METHOD_4LEVEL:
            for(j=1; j<=4; j++) {
                i = (max_order*j/4)-1;
                if(i < 0) i = 0;
                quantize_lpc_coefs(lpc[i], i+1, precision, coefs[i], &shift[i]);
            }
            break;
        case ORDER_METHOD_8LEVEL:
            for(j=1; j<=8; j++) {
                i = (max_order*j/8)-1;
                if(i < 0) i = 0;
                quantize_lpc_coefs(lpc[i], i+1, precision, coefs[i], &shift[i]);
            }
            break;
        case ORDER_METHOD_SEARCH:
            for(i=0; i<max_order; i++) {
                quantize_lpc_coefs(lpc[i], i+1, precision, coefs[i], &shift[i]);
            }
            break;
    }

    return opt_order;
}

int32_t	samples[32],sample=0,coeffs[MAX_LPC_ORDER][MAX_LPC_ORDER],shift[MAX_LPC_ORDER];

int
lpc_parse(char *s)
{
	int a;
			if(sscanf(s,"%d,%d",&a,&samples[sample])==2) {
				if(sample++ == 32) {
					sample=0;
					lpc_calc_coefs(samples,32,8,8,ORDER_METHOD_MAX,coeffs,shift);
				}
	//			plotA=a;
	//			plotB=b/2;		
	//			plotC=c;		
#ifdef	USE_LCD
			if(plot.Refresh())
				lcd.Grid();					
#endif
			return -1;
			}
			return 0;
}
/** 
* @}
*/

