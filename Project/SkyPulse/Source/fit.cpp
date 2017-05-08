#include		"fit.h"
#include		<math.h>
#include		<stdlib.h>

#define 		M_PI	3.141592653589793
/*******************************************************************************
* Function Name	: 
* Description		: Izracun determinante matrike reda n x n, p=element (0,0)
* Output				:
* Return				: None
*******************************************************************************/
double	_FIT::det(double *p, int x,int y, int n) {
				int			sign=1;
				double 	D=0;

				for(int i=1,j=0;j<n;++j,i*= 2) {
					if(!(i & y)) {
						if(x < n-1) {
							D  +=  p[n*j+x]*det(p, x+1, y+i, n)*sign;
							sign *= -1;
						}
						else
							D=p[n*j+x];
					}
				}
				return(D);
}
/*******************************************************************************
* Function Name	: 
* Description		: constructor
* Output				:
* Return				: None
*******************************************************************************/
_FIT::_FIT(int degree, _fittype type) {
				n=degree;
				typ=type;
				tp=new double[4*n*n]();
				fp=new double[2*n]();
				rp=new double[2*n]();		
}
/*******************************************************************************
* Function Name	: 
* Description		: copy constructor
* Output				:
* Return				: None
*******************************************************************************/
_FIT::_FIT(const _FIT &obj)
{
				n=obj.n;
				typ=obj.typ;
				tp=new double[4*n*n]();
				fp=new double[2*n]();
				rp=new double[2*n]();		
}
/*******************************************************************************
* Function Name	: 
* Description		: destructor
* Output				: 
* Return				: None
*******************************************************************************/
_FIT::~_FIT() {
				delete(tp);
				delete(rp);
				delete(fp);
}
/*******************************************************************************
* Function Name	: 
* Description		: add exp. sample
* Output				: 
* Return				: None
*******************************************************************************/
int		_FIT::Sample(double t, double f) {
double	*q = new double(2*n * sizeof(double));
				if(!tp || !fp || !rp)
					return 0;

				switch(typ) {
					case FIT_POW:
						q[0]=1;
						for(int i=1; i<n; ++i)
							q[i]=pow(t,i);
						break;
					case FIT_EXP:
						q[0]=1;
						for(int i=1; i<n; ++i)
							q[i]=exp(pow(t,i));
						break;
					case FIT_NEXP:
						q[0]=1;
						for(int i=1; i<n; ++i)
							q[i]=1.0/exp(pow(t,i));
						break;
					case FIT_TRIG:
						for(int i=0; i < n/2+1; ++i)
							q[i]=cos(2.0*M_PI*t/per*i);
						for(int i=1; i < n/2+1; ++i)
							q[i+n/2]=sin(2.0*M_PI*t/per*i);
						break;
				}
				for(int i=0; i < n; ++i) {
					for(int j=0; j < n ; ++j)
						tp[n*i+j]  +=  q[i]*q[j];
					fp[i]  +=  q[i]*f;
				}
				delete q;
				return tp[0];
}
/*******************************************************************************
* Function Name	: 
* Description		: add trig. sample
* Output				: 
* Return				: None
*******************************************************************************/
int			_FIT::Sample(double t, double f, double period) {
				per=period;
				return Sample(t,f);
}
/*******************************************************************************
* Function Name	: 
* Description		:  Resitev lin. enacbe reda n-1, p= matrika koef. reda n x n
* a[0] + a[1]*p + a[2]*p^2 + ... a[n-1]*p^n-1 = q	
* r = vektor resitev
* ce je DET(p)  ==  0, ni resitve, funkcija vrne NULL
* Output				: 
* Return				: None
*******************************************************************************/
double	*_FIT::Compute() {
				if(!tp || !fp || !rp)
					return(NULL);					
				double	*c,d=det(tp,0,0,n);
				if(!d)
					return(NULL);
				if((int)tp[0] < n)
					return(NULL);

				c=new double(n * n);
				for(int i=0; i<n; ++i) {
					for(int j=0;j< n * n; ++j)
						if(j % n  ==  i)
							c[j]=fp[j / n];
						else
							c[j]=tp[j];
					rp[i]=det(c,0,0,n)/d;
				}
				delete c;
				return(rp);
}
/*******************************************************************************
* Function Name	: 
* Description		: add trig. sample
* Output				: 
* Return				: None
*******************************************************************************/
double	_FIT::Eval(double t) {			
				if(!tp || !fp || !rp)
					return(0);					
				double ft=rp[0];
				switch(typ) {
					case FIT_POW:
						for(int i=1; i<n; ++i)
							ft  +=  rp[i]*pow(t,i);
						break;
					case FIT_EXP:
						for(int i=1; i<n; ++i)
							ft  +=  rp[i]*exp(pow(t,i));
						break;
					case FIT_NEXP:
						for(int i=1; i<n; ++i)
							ft  +=  rp[i]/exp(pow(t,i));
						break;
					case FIT_TRIG:
						for(int i=1; i<n; ++i)
							ft  +=  rp[i]*cos(i*2.0*M_PI*t/per) + rp[i+n-1]*sin(i*2.0*M_PI*t/per);
						break;
				}
				return(ft);
}

